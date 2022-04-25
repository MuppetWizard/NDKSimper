//
// Created by ASUS on 2022/4/25.
//

#ifndef NDKSIMPLE_BASECHANNEL_H
#define NDKSIMPLE_BASECHANNEL_H

#include "JNICallback.h"
#include "../utils/Constans.h"
#include "../utils/safe_queue.h"
extern "C" {
#include "../../ffmpeg/include/libavcodec/avcodec.h"
#include "../../ffmpeg/include/libavutil/time.h"
};

class BaseChannel {
public:
    int stream_index;

    bool isPlaying = 1;
    bool isStop = false;

    AVCodecContext *pContext;
    JNICallback *javaCallHelper;

    //#### 音视频同步需要用到
    //ffmpeg 时间基: 内部时间
    AVRational base_time;
    double audio_time;
    double video_time;
    //#### 音视频同步需要

    //AVPacket 音频: aac 视频: h264
    SafeQueue<AVPacket *> packages; //音频 或者 视频 的压缩数据包(编码的数据包)

    // AVFrame 音频: PCM 视频: YUV
    SafeQueue<AVFrame *> frames; //音频 或 视频 的原始数据包（可直接 渲染 和 播放）

    BaseChannel(int stream_index, AVCodecContext *pContext, AVRational av_base_time, JNICallback *jniCallback){
        this->stream_index = stream_index;
        this->pContext = pContext;
        this->base_time = av_base_time;
        this->javaCallHelper = jniCallback;
        packages.setReleaseCallback(releaseAVPacket);
        frames.setReleaseCallback(releaseAVFrame);
    }

    // 注意：由于是父类，析构函数，必须是虚函数
    virtual ~BaseChannel(){
        packages.clearQueue();
        frames.clearQueue();
    }

    /*
     * 释放 AVPocket 队列
     */
    static void releaseAVPacket(AVPacket **avPacket){
        if (avPacket) {
            av_packet_free(avPacket);
            *avPacket = 0;
        }
    }

    /*
     * 释放 AVFrame 队列
     */
    static void releaseAVFrame(AVFrame **avFrame){
        if (avFrame) {
            av_frame_free(avFrame);
            *avFrame = 0;
        }
    }

    void clear(){
        packages.clearQueue();
        frames.clearQueue();
    }

};

#endif //NDKSIMPLE_BASECHANNEL_H
