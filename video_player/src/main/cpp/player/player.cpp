#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_youyi_player_NativeLib_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_youyi_player_NativeLib_getFFmpegVersion(JNIEnv *env, jobject thiz) {

}
extern "C"
JNIEXPORT void JNICALL
Java_com_youyi_player_NativeLib_setSurfaceNative(JNIEnv *env, jobject thiz, jobject surface) {

}
extern "C"
JNIEXPORT void JNICALL
Java_com_youyi_player_NativeLib_prepareNative(JNIEnv *env, jobject thiz, jstring m_data_source) {

}
extern "C"
JNIEXPORT void JNICALL
Java_com_youyi_player_NativeLib_startNative(JNIEnv *env, jobject thiz) {

}
extern "C"
JNIEXPORT void JNICALL
Java_com_youyi_player_NativeLib_reStartNative(JNIEnv *env, jobject thiz) {

}
extern "C"
JNIEXPORT void JNICALL
Java_com_youyi_player_NativeLib_stopNative(JNIEnv *env, jobject thiz) {

}
extern "C"
JNIEXPORT void JNICALL
Java_com_youyi_player_NativeLib_releaseNative(JNIEnv *env, jobject thiz) {

}
extern "C"
JNIEXPORT void JNICALL
Java_com_youyi_player_NativeLib_isPlayNative(JNIEnv *env, jobject thiz) {

}