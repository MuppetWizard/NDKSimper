//
// Created by ASUS on 2022/4/12.
//

#ifndef NDKSIMPLE_OPENSLAUDIOPLAY_H
#define NDKSIMPLE_OPENSLAUDIOPLAY_H

#include "AudioEngine.h"

class OpenSLAudioPlay {

public:
    OpenSLAudioPlay(int sampleRate, int sampleFormat, int channels);
    ~OpenSLAudioPlay();

    bool init();
private:
    AudioEngine *audioEngine;
    SLObjectItf mPlayerObj;
    SLPlayItf mPlayer;
    SLAndroidSimpleBufferQueueItf mBufferQueue;
    SLEffectSendItf mEffectSend;
    SLVolumeItf mVolume;
    SLmilliHertz mSampleRate;
    int mSampleFormat;
    int mChannels;

    uint8_t *mBuffers[2];
    SLuint32 mBufSize;
    int mIndex;
    pthread_mutex_t mMutex;


};


#endif //NDKSIMPLE_OPENSLAUDIOPLAY_H
