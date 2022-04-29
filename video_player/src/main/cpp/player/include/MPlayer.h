//
// Created by ASUS on 2022/4/22.
//

#ifndef NDKSIMPLE_MPLAYER_H
#define NDKSIMPLE_MPLAYER_H


#include "JNICallback.h"
#include <threads.h>
#include "AudioChannel.h"
#include "VideoChannel.h"


extern "C"{
#include "../../ffmpeg/include/libavformat/avformat.h"
#include "../../ffmpeg/include/libavcodec/avcodec.h"
#include "../../ffmpeg/include/libavutil/time.h"
};

class MPlayer {
public:
    MPlayer();

    MPlayer(const char *dataSource,JNICallback *pCallback);

    ~MPlayer();

    void prepare();

    void prepare_();

    void start();

    void start_();

    void restart();

    void stop();

    void release();

    void setRenderCallback(RenderCallback renderCallback);

    bool isPlaying;

    int getDuration(){
        return duration;
    }

    void seek(int i);
private:
    char *data_source = 0;
    pthread_t pid_prepare;

    AVFormatContext *formatContext = 0;

    AudioChannel *audioChannel = 0;

    VideoChannel *videoChannel = 0;

    JNICallback *pCallback = 0;

    pthread_t pid_start;

    bool isStop = false;

    RenderCallback renderCallback;

    AVCodecContext *codecContext = 0;

    pthread_mutex_t seekMutex;

    int duration = 0;

    bool isSeek = 0;

};


#endif //NDKSIMPLE_MPLAYER_H
