#include "iostream"
#include "functions.h"

const JNINativeMethod nativeMethods[] = {
        {"op", "()I", (void *) native_op}
};

jint native_op(){
    return 1;
}

jint JNI_OnLoad(JavaVM * vm, void * reserved) {
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void **)&env, JNI_VERSION_1_1) == JNI_OK) {
        if (registerNativeMethods(env) == JNI_OK) {
            result = JNI_VERSION_1_6;
        }
    }
    return result;
}

static int registerNativeMethods(JNIEnv *env) {
    int result = -1;
    jclass class_hello = env->FindClass("com/fhj/mvi_demo/BaseActivity");
    if (env->RegisterNatives(class_hello, nativeMethods,
                             sizeof(nativeMethods) / sizeof(nativeMethods[0])) == JNI_OK) {
        result = 0;
    }
    return result;
}