#include <jni.h>
#include <string>
#include "Gles_play.h"

const JavaVM *javaVm = 0;
Gles_play *gles_play = 0;

int JNI_Onload(JavaVM *javaVm, void *pVoid) {
    ::javaVm  = javaVm;
    return JNI_VERSION_1_6; // 坑，这里记得一定要返回，和异步线程指针函数一样（记得返回）
}

extern "C" JNIEXPORT void JNICALL
Java_com_youyi_video_YUVPlay_nativeGlesPlay(JNIEnv *env, jobject thiz, jstring yuv420p_path,
                                            jobject surface) {
  const char *yuv420pPath = env->GetStringUTFChars(yuv420p_path,0);
  PlayCallback *callback = new PlayCallback(const_cast<JavaVM *>(javaVm), env, thiz);
  gles_play = new Gles_play(env,thiz,callback,yuv420pPath,surface);
  //这里prepare 内部会开启一个子线程，由于开启会造成 堆栈溢出 固取消了  JNI 中开启
  gles_play->start();
  env->ReleaseStringUTFChars(yuv420p_path,yuv420pPath);
}

extern "C" JNIEXPORT void JNICALL
Java_com_youyi_video_YUVPlay_nativeWindowPlay(JNIEnv *env, jobject thiz, jstring yuv420p_path,
                                              jobject surface) {
    const char *yuv420pPath = env->GetStringUTFChars(yuv420p_path,0);
    env->ReleaseStringUTFChars(yuv420p_path,yuv420pPath);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_youyi_video_NativeLib_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_youyi_video_YUVPlay_onDestroy(JNIEnv *env, jobject thiz) {
    if (gles_play) {
        gles_play->release();
    }
}