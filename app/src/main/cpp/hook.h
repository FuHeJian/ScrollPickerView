//
// Created by 111 on 2024/1/17.
//
#include <unistd.h>
#include <elf.h>
#include <vector>



#ifndef MVI_DEMO_HOOK_H
#define MVI_DEMO_HOOK_H

#endif //MVI_DEMO_HOOK_H

struct ELF64_GNU_HASH_TABLE {
    Elf64_Addr addr = 0;
    __u32 bucket_size;
    __u32 symbias;
    __u32 indexes_size;
    __u32 shift;
    Elf64_Addr *indexes;
    __u32 *bucket;
    __u32 *chain;
};

struct ELF64INFO {
    Elf64_Addr baseAddr;
    Elf64_Sym *SYMTAB;
    char *STRTAB;
    Elf64_Xword STRTAB_SIZE;
    Elf64_Addr PLTGOT;
    Elf64_Rela *rela;
    Elf64_Xword rela_size;
    bool HAS_GNU_HASH_TABLE = false;
    ELF64_GNU_HASH_TABLE GNUHASHTABLE;

    Elf64_Rela *JMPREL;
    Elf64_Xword JMPREL_SIZE;
};

struct HOOK_INFO{
    void* oldPtr;
};

/**
 * 获取当前so文件的ELF Header
 * @return
 */
Elf64_Ehdr * getSelfHeader();

Elf64_Ehdr * getHeader(char * soName);

/**
 * 返回就函数地址
 * @param segmentInfo
 * @param replaceFunc 替换函数
 * @param symTableIndex 被替换函数索引
 * @param type 被替换的类型
 * @return
 */
uintptr_t hookGotPltItem(ELF64INFO segmentInfo, void *replaceFunc, int symTableIndex, int type);

uint32_t getFunctionSymbolIndexStep2(char *funName, ELF64_GNU_HASH_TABLE hashTable, char *strTab,
                                     Elf64_Sym *symTab);

/**
 * 仅用来快速判断是否存在该函数
 * 存在返回索引，不存在返回ERRNO_NOTFND
 * 只通过gnu hash table获取函数对应的got表，暂时不考虑向前兼容Hash Tbale
 * @return
 */
uint32_t getFunctionSymbolIndex(char *funName, ELF64_GNU_HASH_TABLE hashTable, char *strTab,
                                Elf64_Sym *symTab);

/**
 * 解析ELF_GNU_HASH_TABLE数据
 * @param info
 */
void initGNUHASHTABLE(ELF64_GNU_HASH_TABLE *info);

Elf64_Phdr getDynamicPHT(Elf64_Ehdr *headerInfo);

uint32_t elf_gnu_hash(const uint8_t *name);

ELF64INFO getELF64INFO(Elf64_Addr baseAddr, Elf64_Phdr dynamic);

uint32_t findFuncPtr(ELF64INFO segmentInfo, int symTableIndex);

std::vector<char *>& logFunc(ELF64INFO segmentInfo, char *searchName);

/**
 * 打印32位的so文件信息
 * @return
 */
std::vector<std::string> logSo32();

std::vector<std::string> logAllSo();