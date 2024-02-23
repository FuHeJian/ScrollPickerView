//
// Created by 111 on 2024/1/23.
//

#ifndef MVI_DEMO_VIDEO_PLAYER_H
#define MVI_DEMO_VIDEO_PLAYER_H

#include "jni.h"

void startPlay(JNIEnv *env, jobject thiz, jobject surface, char *path);
jint loadVideo(JNIEnv *env, jobject thiz, jobject surface, jstring jPath);
jstring parseErrorCode(JNIEnv *env, jobject thiz, jint errorCode);
void stopPlaying(JNIEnv *env, jobject thiz, jobject surface);
void init(JNIEnv *env, jobject thiz);

#endif //MVI_DEMO_VIDEO_PLAYER_H
