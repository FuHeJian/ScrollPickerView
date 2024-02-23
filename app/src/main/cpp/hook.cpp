//
// 调用过程
//

#include <stdlib.h>
#include <stdio.h>
#include <filesystem>
#include <sys/mman.h>
#include <inttypes.h>
#include "dlfcn.h"
#include "stdexcept"
#include "string"
#include "hook.h"
#include <vector>
#include <link.h>

using namespace std;

#define PAGE_START(addr) ((addr) & PAGE_MASK)
#define PAGE_END(addr)   (PAGE_START(addr + sizeof(uintptr_t) - 1) + PAGE_SIZE)
#define PAGE_COVER(addr) (PAGE_END(addr) - PAGE_START(addr))
#define ERRNO_NOTFND -1

uintptr_t hookGotPltItem(ELF64INFO segmentInfo, void *replaceFunc, int symTableIndex, int type) {
    int bind = 0;
    int _t = 0;
    if (symTableIndex < 0 || type < 0)return ERRNO_NOTFND;
//    bool isWeak  = true;
    Elf64_Sym tItem = segmentInfo.SYMTAB[symTableIndex];
    bind = ELF64_ST_BIND(tItem.st_info);
    _t = ELF64_ST_TYPE(tItem.st_info);
    int index = -1;
    if (type == STT_FUNC) {//函数类型，去JMP表中寻找
        for (int i = 0; i < segmentInfo.JMPREL_SIZE; i++) {
            Elf64_Rela item = segmentInfo.JMPREL[i];
            index = ELF64_R_SYM(item.r_info);
            if (_t == type && index == symTableIndex) {
                uintptr_t *ptr = (uintptr_t *) (segmentInfo.baseAddr + item.r_offset);
                //由于我们要修改的地方是只读的，所以要强制设置成可写，否则会由于权限问题崩溃。
                uintptr_t v = (uintptr_t) ptr;
                uintptr_t old = *ptr;
                int re = mprotect((void *) PAGE_START(v), PAGE_COVER(v), PROT_READ | PROT_WRITE);
                if (!re) {
                    (*ptr) = reinterpret_cast<uintptr_t>(replaceFunc);
                }
                __builtin___clear_cache((char *) PAGE_START(v), (char *) PAGE_END(v));
                return old;
            } else {
                continue;
            }
        }
    } else {
        for (int i = 0; i < segmentInfo.rela_size; i++) {
            Elf64_Rela item = segmentInfo.rela[i];
            index = ELF64_R_SYM(item.r_info);
            if (_t == type && index == symTableIndex) {
                uintptr_t *ptr = (uintptr_t *) (segmentInfo.baseAddr + item.r_offset);
                uintptr_t v = (uintptr_t) ptr;
                uintptr_t old = *ptr;
                int re = mprotect((void *) PAGE_START(v), PAGE_COVER(v), PROT_READ | PROT_WRITE);
                if (!re) {
                    (*ptr) = reinterpret_cast<uintptr_t>(replaceFunc);
                }
                return old;
            } else {
                continue;
            }
        }
    }

    return ERRNO_NOTFND;
}

uint32_t getFunctionSymbolIndex(char *funName, ELF64_GNU_HASH_TABLE hashTable, char *strTab,
                                Elf64_Sym *symTab) {
    uint32_t hash = elf_gnu_hash((uint8_t *) funName);

    static uint32_t elfclass_bits = sizeof(Elf64_Addr) * 8;
    size_t word = hashTable.indexes[(hash / elfclass_bits) % hashTable.indexes_size];
    size_t mask = 0
                  | (size_t) 1 << (hash % elfclass_bits)
                  | (size_t) 1 << ((hash >> hashTable.shift) % elfclass_bits);

    //if at least one bit is not set, this symbol is surely missing
    if ((word & mask) != mask)
        return getFunctionSymbolIndexStep2(funName, hashTable, strTab, symTab);

    //ignore STN_UNDEF
    uint32_t i = hashTable.bucket[hash % hashTable.bucket_size];
    if (i < hashTable.symbias)
        return getFunctionSymbolIndexStep2(funName, hashTable, strTab, symTab);

    //loop through the chain
    while (1) {
        const char *symname = (char *) (strTab + symTab[i].st_name);
        const uint32_t symhash = hashTable.chain[i - hashTable.symbias];

        if ((hash | (uint32_t) 1) == (symhash | (uint32_t) 1) && 0 == strcmp(funName, symname)) {
            return i;
        }

        //chain ends with an element with the lowest bit set to 1
        if (symhash & (uint32_t) 1) break;

        i++;
    }

    return ERRNO_NOTFND;
}

uint32_t getFunctionSymbolIndexStep2(char *funName, ELF64_GNU_HASH_TABLE hashTable, char *strTab,
                                     Elf64_Sym *symTab) {
    for (int i = 0; i < hashTable.symbias; i++) {
        Elf64_Sym item = symTab[i];
        char *symname = strTab + item.st_name;
        if (0 == strcmp(funName, symname)) {
            return i;
        }
    }
    return ERRNO_NOTFND;
}

uint32_t elf_gnu_hash(const uint8_t *name) {
    uint32_t h = 5381;

    while (*name != 0) {
        h += (h << 5) + *name++;
    }
    return h;
}

Elf64_Phdr getDynamicPHT(Elf64_Ehdr *headerInfo) {
    Elf64_Off offset = headerInfo->e_phoff;
    Elf64_Half size = headerInfo->e_phnum;
    Elf64_Phdr *phdrs = (Elf64_Phdr *) (offset + (Elf64_Addr) headerInfo);
    for (int i = 0; i < size; i++) {
        if (phdrs[i].p_type == PT_DYNAMIC) {
            return phdrs[i];
        }
    }
    throw runtime_error("无法获取到动态段");
}

void initGNUHASHTABLE(ELF64_GNU_HASH_TABLE *info) {
    if (info->addr == 0) {
        throw runtime_error("未获取GNUHASHTABLE的地址");
    }
    info->bucket_size = *(__u32 *) info->addr;

    info->symbias = *(((__u32 *) info->addr) + 1);

    info->indexes_size = *(((__u32 *) info->addr) + 2);

    info->shift = *(((__u32 *) info->addr) + 3);

    info->indexes = (Elf64_Addr *) (((__u32 *) info->addr) + 4);

    info->bucket = (__u32 *) (&info->indexes[info->indexes_size]);

    info->chain = (__u32 *) (&info->bucket[info->bucket_size]);

}

ELF64INFO getELF64INFO(Elf64_Addr baseAddr, Elf64_Phdr dynamic) {
    ELF64INFO info;
    Elf64_Dyn *dynItems = (Elf64_Dyn *) (dynamic.p_vaddr + baseAddr);
    info.baseAddr = baseAddr;
    int size = dynamic.p_memsz / sizeof(Elf64_Dyn);
    for (int i = 0; i < size; i++) {
        if (dynItems[i].d_tag == DT_PLTGOT) {
            info.PLTGOT = baseAddr + dynItems[i].d_un.d_ptr;
        } else if (dynItems[i].d_tag == DT_SYMTAB) {
            info.SYMTAB = (Elf64_Sym *) (baseAddr + dynItems[i].d_un.d_ptr);
        } else if (dynItems[i].d_tag == DT_STRTAB) {
            info.STRTAB = (char *) (baseAddr + dynItems[i].d_un.d_ptr);
        } else if (dynItems[i].d_tag == DT_STRSZ) {
            info.STRTAB_SIZE = dynItems[i].d_un.d_val;
        } else if (dynItems[i].d_tag == DT_GNU_HASH) {
            info.GNUHASHTABLE.addr = baseAddr + dynItems[i].d_un.d_ptr;
            info.HAS_GNU_HASH_TABLE = true;
            initGNUHASHTABLE(&info.GNUHASHTABLE);
        } else if (dynItems[i].d_tag == DT_RELA) {
            info.rela = (Elf64_Rela *) (baseAddr + dynItems[i].d_un.d_ptr);
        } else if (dynItems[i].d_tag == DT_RELASZ) {
            info.rela_size = dynItems[i].d_un.d_val / sizeof(Elf64_Rela);
        } else if (dynItems[i].d_tag == DT_JMPREL) {
            info.JMPREL = (Elf64_Rela *) (baseAddr + dynItems[i].d_un.d_ptr);
        } else if (dynItems[i].d_tag == DT_PLTRELSZ) {
            info.JMPREL_SIZE = dynItems[i].d_un.d_val / sizeof(Elf64_Rela);
        }
    }
    return info;
}

Elf64_Ehdr *getSelfHeader() {
    Dl_info info;
    dladdr((void *) getSelfHeader, &info);
    Elf64_Ehdr *headerInfo = (Elf64_Ehdr *) info.dli_fbase;
    return headerInfo;
}

Elf64_Ehdr *getHeader(char *soName) {

    FILE *fd = fopen(("/proc/" + to_string(getpid()) + "/maps").c_str(), "r");
    size_t fl = 0;
    char buf[512];
    uintptr_t base_addr = 0;
    if (fd != nullptr) {
        while (fgets(buf, sizeof(buf), fd)) {
            if (strstr(buf, soName) != nullptr && strstr(buf, "00000000")) {
                sscanf(buf, "%" PRIxPTR "-%*lx %*4s 00000000", &base_addr);
                if (base_addr != 0)break;
            }
        }
        fclose(fd);
    }

    if (base_addr != 0) {
        Elf64_Ehdr *info = (Elf64_Ehdr *) base_addr;
        return info;
    } else {
        return nullptr;
    }

}

uint32_t findFuncPtr(ELF64INFO segmentInfo, int symTableIndex) {
    int bind = 0;
    int _t = 0;
    if (symTableIndex < 0)return ERRNO_NOTFND;
//    bool isWeak  = true;
    Elf64_Sym tItem = segmentInfo.SYMTAB[symTableIndex];
    bind = ELF64_ST_BIND(tItem.st_info);
    _t = ELF64_ST_TYPE(tItem.st_info);
    int index = -1;
    for (int i = 0; i < segmentInfo.JMPREL_SIZE; i++) {
        Elf64_Rela item = segmentInfo.JMPREL[i];
        index = ELF64_R_SYM(item.r_info);
        if (_t == STT_FUNC && index == symTableIndex) {
            uintptr_t *ptr = (uintptr_t *) (segmentInfo.baseAddr + item.r_offset);
            uintptr_t v = (uintptr_t) ptr;
            uintptr_t old = *ptr;
            return old;
        } else {
            continue;
        }
    }
    return ERRNO_NOTFND;
}

vector<char *> &logFunc(ELF64INFO segmentInfo, char *searchName) {
    int index = 0;
    int _t = 0;
    vector<char *> list;
    for (int i = 0; i < segmentInfo.JMPREL_SIZE; i++) {
        Elf64_Rela item = segmentInfo.JMPREL[i];
        index = ELF64_R_SYM(item.r_info);
        _t = ELF64_ST_TYPE(segmentInfo.SYMTAB[index].st_info);
        char *name = segmentInfo.STRTAB + segmentInfo.SYMTAB[index].st_name;
        if (_t == STT_FUNC && strstr(name, searchName)) {
            list.push_back(name);
        } else {
            continue;
        }
    }

    return list;
}

vector<string> logAllSo() {

    FILE *fd = fopen(("/proc/" + to_string(getpid()) + "/maps").c_str(), "r");
    size_t fl = 0;
    char buf[512];
    auto info = vector<string>();
    uintptr_t base_addr = 0;

    if (fd != nullptr) {
//
        while (fgets(buf, sizeof(buf), fd)) {
            if (strstr(buf, ".so") && strstr(buf, "00000000")) {
                base_addr = 0;
                sscanf(buf, "%" PRIxPTR "-%*lx %*4s 00000000", &base_addr);
                if (base_addr) {
                    string* aa = new string(buf);
                    info.push_back(*aa);
                }
            }
        }

        fclose(fd);

    }

    return info;

}

vector<string> logSo32() {

    FILE *fd = fopen(("/proc/" + to_string(getpid()) + "/maps").c_str(), "r");
    size_t fl = 0;
    char buf[512];
    auto info = vector<string>();
    uintptr_t base_addr = 0;

    if (fd != nullptr) {
//
        while (fgets(buf, sizeof(buf), fd)) {
            if (strstr(buf, ".so") && strstr(buf, "00000000")) {
                base_addr = 0;
                sscanf(buf, "%" PRIxPTR "-%*lx %*4s 00000000", &base_addr);
                if(base_addr){
                    Elf64_Ehdr *headerInfo = (Elf64_Ehdr *) base_addr;
                    if (headerInfo->e_ident[EI_CLASS] == ELFCLASS32) {
                        string* aa = new string(buf);
                        info.push_back(*aa);
                    }
                }
            }
        }

        fclose(fd);

    }

    return info;

}