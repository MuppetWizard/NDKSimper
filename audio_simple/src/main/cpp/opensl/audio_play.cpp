//
// Created by ASUS on 2022/4/12.
//
#include <jni.h>
#include <pthread.h>
#include "OpenSLAudioPlay.h"

/*
 * 播放file
 */
FILE *pcmFile = 0;

OpenSLAudioPlay *slAudioPlay = nullptr;

/*
 * 是否正在播放
 */
bool isPlaying = false;

void *playThreadFunc(void *arg);

void *playThreadFunc(void *arg){
    const int bufferSize = 2048;
    short buffer[bufferSize];
    while (isPlaying && !feof(pcmFile)) {
        fread(buffer,1,bufferSize,pcmFile);
        slAudioPlay->enqueueSample(buffer,bufferSize);
    }
    return 0;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_youyi_audio_MainActivity_nativePlayPCM(JNIEnv *env, jobject thiz, jstring pcmPath_) {
    //将 string 转为 char
    const char * _pcmPath = env->GetStringUTFChars(pcmPath_,NULL);

    //如果已经实例化则释放资源
    if (slAudioPlay) {
        slAudioPlay->release();
        delete slAudioPlay;
        slAudioPlay = nullptr;
    }

    //实例化OpenSLAudioPlay
    slAudioPlay = new OpenSLAudioPlay(44100,SAMPLE_FORMAT_16,1);
    slAudioPlay->init();
    pcmFile = fopen(_pcmPath,"r");
    isPlaying = true;
    pthread_t playThread;
    pthread_create(&playThread, nullptr, playThreadFunc,0);

    env -> ReleaseStringUTFChars(pcmPath_,_pcmPath);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_youyi_audio_MainActivity_nativeStopPCM(JNIEnv *env, jobject thiz) {
    isPlaying = false;
    if (slAudioPlay) {
        slAudioPlay->release();
        delete slAudioPlay;
        slAudioPlay = nullptr;
    }
    if (pcmFile) {
        fclose(pcmFile);
        pcmFile = nullptr;
    }
}