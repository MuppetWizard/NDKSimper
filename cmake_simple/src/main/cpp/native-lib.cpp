//
// Created by ASUS on 2022/3/30.
//
#include <jni.h>
#include <string>
#include <android/log.h>

extern "C" {
    int main();
}

extern "C" JNIEXPORT void JNICALL
Java_com_youyi_cmake_1simple_MainActivity_testCmake(JNIEnv *env, jobject thiz) {
    std::string hello = "Hello World From C++";
    __android_log_print(ANDROID_LOG_DEBUG, "Muppet", "main--->:%d", main());
}