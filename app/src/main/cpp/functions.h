//
// Created by 111 on 2023/10/7.
//
#include "jni.h"
#include <android/native_window.h>
#ifndef MVI_DEMO_FUNCTIONS_H
#define MVI_DEMO_FUNCTIONS_H

#endif //MVI_DEMO_FUNCTIONS_H

jint native_op(JNIEnv *env, jobject thiz,jstring p,jobject oo);
static int registerNativeMethods(JNIEnv *env);
jlong openVideo(char * path,ANativeWindow * nativeWindow);