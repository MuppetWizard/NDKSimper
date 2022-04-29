#include <jni.h>
#include <string>
#include "include/JNICallback.h"
#include "include/MPlayer.h"
#include <android/native_window_jni.h>

extern "C" {
#include <libavutil/avutil.h>
}

JavaVM *javaVm = 0;
MPlayer *player = 0;
ANativeWindow *nativeWindow = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //静态初始化 互斥锁

int JNI_OnLoad(JavaVM *javaVm, void *pVoid){
    ::javaVm = javaVm;
    // 坑，这里记得一定要返回，和异步线程指针函数一样（记得返回）
    return JNI_VERSION_1_6;
}

/**
 * @return 返回 FFmpeg版本
 */
const char *getFFmpegVersion(){
    return av_version_info();
}

/**
 * 负责渲染的函数
 * @param src_data  解码后的视频 rgba 数据
 * @param width 视频宽
 * @param height 视频高
 * @param src_size 行数 size 相关信息
 */
void renderFrame(uint8_t *src_data, int width, int height, int src_size){
    pthread_mutex_lock(&mutex);
    if (!nativeWindow) {
        pthread_mutex_unlock(&mutex);
        nativeWindow = 0;
        return;
    }

    //设置窗口属性
    ANativeWindow_setBuffersGeometry(nativeWindow, width, height, WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer window_buffer;

    if (ANativeWindow_lock(nativeWindow, &window_buffer, 0)) {
        ANativeWindow_release(nativeWindow);
        nativeWindow = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }

    //填数据到buffer 就是修改数据
    uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    int lineSize  = window_buffer.stride * 4; //RGBA

    //逐行copy
    //一行 copy
    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst_data + i * lineSize,src_data + i * src_size,lineSize);
    }

}

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