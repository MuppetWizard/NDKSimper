#include <jni.h>
#include <string>
#include "Gles_play.h"

const JavaVM *javaVm = 0;
Gles_play *gles_play = 0;

int JNI_Onload(JavaVM *javaVm, void *pVoid) {
    ::javaVm  = javaVm;
    return JNI_VERSION_1_6; // 坑，这里记得一定要返回，和异步线程指针函数一样（记得返回）
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_youyi_video_NativeLib_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

