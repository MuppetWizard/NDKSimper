//
// Created by ASUS on 2022/4/14.
//

#include <pthread.h>
#include "Gles_play.h"

EGLDisplay display;
ANativeWindow *nativeWindow = 0;
int width = 640;
int height = 272;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 静态初始化 互斥锁


void Gles_play::playYUV(jobject surface) {
    //加锁
    pthread_mutex_lock(&mutex);
    //判断资源是否被释放
    release();

    showMessage(env,"start", true);

    //开始播放标志
    isPlay = true;

    //1、获取原始窗口
    nativeWindow = ANativeWindow_fromSurface(env,surface);

    //获取 Display
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if (display == EGL_NO_DISPLAY) {
        LOGD("egl display failed");
        showMessage(env,"egl display failed", false);
        return;
    }

    //初始化egl, 后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display,0,0)) {
        LOGD("egl Initialize failed");
        showMessage(env, "egl Initialize failed", false);
        return;
    }

    //3.1 surface 配置
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE,8,
            EGL_GREEN_SIZE,8,
            EGL_BLUE_SIZE,8,
            EGL_SURFACE_TYPE,EGL_WINDOW_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        LOGD("egl ChooseConfig failed");
        showMessage(env,"egl ChooseConfig failed", false);
        return;
    }



}

void Gles_play::start() {

}

void Gles_play::progress() {

}

void Gles_play::release() {

}



Gles_play::Gles_play(JNIEnv *env, jobject thiz, PlayCallback *playCallback, const char *data_source,
                     jobject surface) {
    this->playCallback = playCallback;
    this->env = env;
    this->thiz = thiz;
    this->surface = surface;
    this->data_source = new char[strlen(data_source) + 1];
    strcpy(this->data_source,data_source);
}

Gles_play::~Gles_play() {
    if (playCallback) {
        delete playCallback;
        playCallback = 0;
    }
}