//
// Created by ASUS on 2022/4/14.
//

#ifndef NDKSIMPLE_GLES_PLAY_H
#define NDKSIMPLE_GLES_PLAY_H

//使用 EGL 需要添加的头文件
#include <EGL/egl.h>
#include <EGL/eglext.h>

//使用 OpenGL ES 2.0 需要添加的头文件
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <string.h>
#include "PlayCallback.h"

#define LOGD(...) __android_log_print(ANDROID_LOG_WARN,"DevYK",__VA_ARGS__)

class Gles_play {
public:
    Gles_play(JNIEnv *env,jobject thiz, PlayCallback *playCallback, const char *data_source, jobject pjobject);

    ~Gles_play();

    void playYUV(jobject surface);

    void release();

    void prepare();

    void start();

private:
    PlayCallback *playCallback = 0;

    void showMessage(JNIEnv *, const char *,bool );

    JNIEnv *env = 0;
    jobject thiz;
    pthread_t pid_prepare;
    char *data_source = 0;
    jobject surface;
    bool isPlay;


};


#endif //NDKSIMPLE_GLES_PLAY_H
