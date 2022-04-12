//
// Created by ASUS on 2022/4/12.
//
#include <pthread.h>
#include <android/log.h>
#include "OpenSLAudioPlay.h"

#define TAG "OpenSL Play"
#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,TAG,FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,TAG,FORMAT,##__VA_ARGS__);


OpenSLAudioPlay::OpenSLAudioPlay(int sampleRate, int sampleFormat, int channels)
: audioEngine(new AudioEngine()),mPlayerObj(nullptr), mPlayer(nullptr), mBufferQueue(nullptr),
        mEffectSend(nullptr), mVolume(nullptr), mSampleRate((SLmilliHertz) sampleRate * 1000),
        mSampleFormat(sampleFormat), mChannels(channels), mBufSize(0), mIndex(0){
    mMutex = PTHREAD_MUTEX_INITIALIZER;
    mBuffers[0] = nullptr;
    mBuffers[1] = nullptr;
}

OpenSLAudioPlay::~OpenSLAudioPlay() {
}

bool OpenSLAudioPlay::init(){
    SLresult result;
    //第三步，创建播放器
    //3.1 配置输入声音信息
    //创建buffer缓冲类型的队列，2个队列
    SLDataLocator_AndroidSimpleBufferQueue locBufQ = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};

    // pcm数据格式
    // SL_DATAFORMAT_PCM：数据格式为pcm格式
    // 2：双声道
    // SL_SAMPLINGRATE_44_1：采样率为44100（44.1赫兹 应用最广的，兼容性最好的）
    // SL_PCMSAMPLEFORMAT_FIXED_16：采样格式为16bit （16位）(2个字节)
    // SL_PCMSAMPLEFORMAT_FIXED_16：数据大小为16bit （16位）（2个字节）
    // SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT：左右声道（双声道）  （双声道 立体声的效果）
    // SL_BYTEORDER_LITTLEENDIAN：小端模式
    SLDataFormat_PCM formatPcm = {SL_DATAFORMAT_PCM,(SLuint32) mChannels,mSampleRate,
                                  (SLuint32) mSampleFormat,(SLuint32) mSampleFormat,
                                  mChannels == 2 ? 0 : SL_SPEAKER_FRONT_CENTER,
                                  SL_BYTEORDER_LITTLEENDIAN};

    /*
     * Enable Fast Audio when possible:  once we set the same rate to be the native, fast audio path
     * will be triggered
     */
    if (mSampleRate) {
        formatPcm.samplesPerSec = mSampleRate;
    }

    //数据源 将配置信息放入数据源中
    SLDataSource audioSrc = {&locBufQ,&formatPcm};

    //3.2 配置音轨(输出)
    //设置混音器
    SLDataLocator_OutputMix locOutputMix = {SL_DATALOCATOR_OUTPUTMIX,audioEngine->outputMixObj};
    SLDataSink audioSink = {&locOutputMix, nullptr};

    /*
     * Create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    //所需的接口 操作队列的接口
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE,SL_IID_VOLUME,SL_IID_EFFECTSEND};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE};
}