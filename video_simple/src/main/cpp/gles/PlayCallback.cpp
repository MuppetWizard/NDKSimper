//
// Created by ASUS on 2022/4/14.
//

#include "PlayCallback.h"

PlayCallback::~PlayCallback() {  }

PlayCallback::PlayCallback(JavaVM *javaVM, JNIEnv *env, jobject job) {
    this->javaVm = javaVM;
    this->env = env;
    this->instance = env->NewGlobalRef(job);
}

void PlayCallback::toJavaMessage(const char *message) {
    //拿到 jclass
    jclass videoPlayClass = this->env->GetObjectClass(instance);
    //拿到 method id
    this->jmd_showMessage = this->env->GetMethodID(videoPlayClass,"showMessage","(Ljava/lang/String;)V");
    jstring string = env->NewStringUTF(message);
    //通过反射执行java方法
    this->env->CallVoidMethod(instance,jmd_showMessage,string)
}

void PlayCallback::onSucceed(const char *message) {
    toJavaMessage(message);
}

void PlayCallback::onError(const char *message) {
    toJavaMessage(message);
}
