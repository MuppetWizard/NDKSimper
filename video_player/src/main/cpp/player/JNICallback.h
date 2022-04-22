//
// Created by ASUS on 2022/4/22.
//

#ifndef NDKSIMPLE_JNICALLBACK_H
#define NDKSIMPLE_JNICALLBACK_H

#include <jni.h>
#include "utils/Constans.h"

class JNICallback {
public:
    JNICallback(JavaVM *javaVm, JNIEnv *env, jobject instance);

    //回调
    void onPrepared(int thread_mod);
    void onErrorAction(int thread_mod, int error_code);
    void onProgress(int thread, int progress);

    ~JNICallback();
    


private:
    JavaVM *javaVm = 0;
    JNIEnv *env = 0;
    jobject instance;

    //相当于反射拿到 Java 函数
    jmethodID jmd_prepared;
    jmethodID jmd_error;
    jmethodID jmid_progress;
};


#endif //NDKSIMPLE_JNICALLBACK_H
