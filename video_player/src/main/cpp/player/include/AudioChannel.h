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
    int out_channel;
    int out_sample_size;
    int out_sample_rate;
    int out_buffers_size;

    AudioChannel(int stream_index, AVCodecContext *pContext, AVRational , JNICallback *jniCallback);

    ~AudioChannel();

    void stop();

    void start();

    void reStart();

    void audio_decode();

    void audio_player();

    void release();

    int getPCM();

    //定义一个缓冲区
    uint8_t *out_buffers = 0;



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
    // 获取播放器队列接口
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

    SwrContext *swr_context;

};


#endif //NDKSIMPLE_AUDIOCHANNEL_H
