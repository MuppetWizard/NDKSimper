//
// Created by ASUS on 2022/4/14.
//

#ifndef NDKSIMPLE_PLAYCALLBACK_H
#define NDKSIMPLE_PLAYCALLBACK_H

#include <jni.h>

class PlayCallback {
public:
    PlayCallback(JavaVM *javaVM, JNIEnv *env, jobject job);

    ~PlayCallback();

    void onSucceed(const char *);

    void onError(const char *);

    void toJavaMessage(const char *message);

private:
    JavaVM *javaVm = 0;
    JNIEnv *env = 0;
    jobject instance;

    jmethodID jmd_showMessage;

};


#endif //NDKSIMPLE_PLAYCALLBACK_H
