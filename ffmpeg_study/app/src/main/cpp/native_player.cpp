//
// Created by hyk on 21-9-1.
//
#include <cstdio>
#include <cstring>
#include "util/LogUtil.h"
#include "jni.h"
#include "player/AvPlayer.h"
//由于 FFmpeg 库是 C 语言实现的，告诉编译器按照 C 的规则进行编译


#define TAG = "Native_Player"

extern "C"
{

JNIEXPORT jlong JNICALL
Java_com_ykhe_ffmpeg_1study_player_AVPlayer_nativeInit(JNIEnv *env, jobject thiz) {
    auto* avPlayer = new AvPlayer();

    return reinterpret_cast<jlong>(avPlayer);
}


JNIEXPORT void JNICALL
Java_com_ykhe_ffmpeg_1study_player_AVPlayer_setDataSource(JNIEnv *env, jobject thiz,
                                                          jlong native_handle, jstring path) {
        auto *avplay = reinterpret_cast<AvPlayer *>(native_handle);
        const char* path_ = env->GetStringUTFChars(path, nullptr);
        avplay->setDataSource(path_);
        env->ReleaseStringUTFChars(path,path_);

}

JNIEXPORT void JNICALL
Java_com_ykhe_ffmpeg_1study_player_AVPlayer_prepare(JNIEnv *env, jobject thiz,
                                                    jlong native_handle) {
    auto *avplay = reinterpret_cast<AvPlayer *>(native_handle);
    avplay->prepare();
}
}
