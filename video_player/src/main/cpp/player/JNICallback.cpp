//
// Created by ASUS on 2022/4/22.
//

#include "include/JNICallback.h"

JNICallback::JNICallback(JavaVM *javaVm, JNIEnv *env, jobject instance) {
    this->javaVm = javaVm;
    this->env = env;
    // jobject 一旦涉及到跨函数，跨线程，就必须是全局引用
    this->instance = env->NewGlobalRef(instance);
    // 拿到java对象的 class
    jclass mMPlayerClass = env->GetObjectClass(this->instance);
    //声明 Java onPrepared/onError 回调签名
    const char *signPre = "()V"; //空函数无返回值
    const char *signErr = "(I)V"; //int 参数 无返回值
    const char *signPro = "(I)V"; //int 参数 无返回值

    this->jmd_prepared = env->GetMethodID(mMPlayerClass,"onPrepared",signPre);
    this->jmid_progress = env->GetMethodID(mMPlayerClass,"onProgress",signPro);
    this->jmd_error = env->GetMethodID(mMPlayerClass,"onError",signErr);

}

void JNICallback::onPrepared(int thread_mod) {
    if (thread_mod == THREAD_MAIN) {
        //主线程可以直接调用 Java 方法
        env->CallVoidMethod(this->instance,jmd_prepared);
    } else {
        //子线程 用附加 native 线程到 JVM 的方式，来获取权限env
        JNIEnv *jniEnv = nullptr;
        jint result = javaVm->AttachCurrentThread(&jniEnv,0);
        if (result != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(this->instance,this->jmd_prepared);
        javaVm->DetachCurrentThread();
    }
}

void JNICallback::onErrorAction(int thread_mod, int error_code) {
    if (thread_mod == THREAD_MAIN) {
        env->CallVoidMethod(this->instance,this->jmd_error,error_code);
    } else{
        JNIEnv *jniEnv = nullptr;
        jint result = javaVm->AttachCurrentThread(&jniEnv,0);
        if (result != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(this->instance,this->jmd_error,error_code);
        javaVm->DetachCurrentThread();
    }
}

void JNICallback::onProgress(int thread, int progress) {
    if (thread == THREAD_CHILD) {
        JNIEnv *jniEnv = nullptr;
        if (javaVm->AttachCurrentThread(&jniEnv,0)){
            return;
        }
        jniEnv->CallVoidMethod(this->instance,this->jmid_progress,progress);
        javaVm->DetachCurrentThread();
    } else{
        env->CallVoidMethod(this->instance,this->jmid_progress,progress);
    }
}

JNICallback::~JNICallback() {
    LOGD("~JNICallback");
    this->javaVm = 0;
    env->DeleteGlobalRef(this->instance);
    this->instance = 0;
    env = 0;

}


