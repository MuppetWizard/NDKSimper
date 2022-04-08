//
// Created by ASUS on 2022/4/8.
//
#include <jni.h>

// 有坑，会报错，必须混合编译
//#include <libavutil/avutil.h>

extern "C" {
#include <libavutil/avutil.h>
}


/**
 * 拿到 ffmpeg 当前版本
 * @return
 */
const char *getFFmpegVer() {
    return av_version_info();
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_youyi_ffmpeg_MainActivity_00024Companion_getFFmpegVersion(JNIEnv *env, jobject thiz) {
    return env->NewStringUTF(getFFmpegVer());
}