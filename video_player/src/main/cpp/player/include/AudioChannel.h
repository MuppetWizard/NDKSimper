//
// Created by ASUS on 2022/4/25.
//

#ifndef NDKSIMPLE_AUDIOCHANNEL_H
#define NDKSIMPLE_AUDIOCHANNEL_H

#include "BaseChannel.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h> // 重采样支持
};

class AudioChannel : public BaseChannel {
public:
    AudioChannel(int stream_index, AVCodecContext *pContext, AVRational , JNICallback *jniCallback);
    ~AudioChannel();

    void stop();

    void start();

    void audio_decode();

    void audio_player();

    int getPCM();

private:
    //线程 id
    pthread_t pid_audio_decode;
    pthread_t pid_audio_player;

    // 引擎
    SLObjectItf engineObject;
    // 引擎接口
    SLEngineItf engineInterface;
    // 混音器
    SLObjectItf outputMixObject;
    // 播放器的
    SLObjectItf bqPlayerObjct;
    // 播放器接口
    SLPlayItf bqPlayerPlay;





};


#endif //NDKSIMPLE_AUDIOCHANNEL_H
