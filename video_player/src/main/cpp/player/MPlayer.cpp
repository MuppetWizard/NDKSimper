//
// Created by ASUS on 2022/4/22.
//

#include "MPlayer.h"
#include <unistd.h>

MPlayer::MPlayer() {
}

MPlayer::MPlayer(const char *dataSource, JNICallback *pCallback) {
    this->data_source = new char[strlen(dataSource) + 1];
    strcpy(this->data_source, dataSource);
    this->pCallback = pCallback;
    duration = 0;
    pthread_mutex_init(&seekMutex,0);
}

MPlayer::~MPlayer() {
}

void *customTaskStartThread(void *pVoid){
    MPlayer *mPlayer = static_cast<MPlayer *>(pVoid);
    mPlayer->start();
    return 0;
}

void MPlayer::prepare() {
    //解析流媒体，通过ffmpeg API来解析 dataSource
    //这里需要异步，由于这个函数从 java 主线程调用，所以这里需要创建一个异步线程
    pthread_create(&pid_prepare,0,customTaskStartThread, this);

}

void MPlayer::prepare_() {
    LOGD("第一步，打开流媒体地址");
    //1,打开流媒体地址(文件路径、直播地址)
    // 可以初始为NULL，如果初始为NULL，当执行avformat_open_input函数时，内部会自动申请avformat_alloc_context，这里干脆手动申请
    //封装了媒体流的格式信息
    formatContext = avformat_alloc_context();

    //字典，键值对
    AVDictionary *dictionary = 0;
    av_dict_set(&dictionary,"timeout","5000000",0);//单位是微妙


    /*
     * @param AVFormatContext: 传入一个 format 上下文是一个二级指针
     * @param const char *url: 播放源
     * @param ff_const59 AVInputFormat *fmt: 输入的封住格式，一般让 ffmpeg 自己去检测，所以给了一个 0
     * @param AVDictionary **options: 字典参数
     */
    int result = avformat_open_input(&formatContext,data_source,0,&dictionary);
    //result -13--> 没有读写权限
    //result -99--> 第三个参数写 NULl
    LOGD("avformat_open_input-->  %d, %s",result,data_source);
    //释放字典
    av_dict_free(&dictionary);

    if (result) {//0 on success true
        //文件路径，或文件损坏
        //将错误信息，通知到Java层
        if (pCallback) {
            pCallback->onErrorAction(THREAD_CHILD,FFMPEG_CAN_NOT_OPEN_URL);
        }
        return;
    }

    //第二步 查找媒体中的音视频流的信息
    LOGD("第二步，查找媒体中的音视频流的信息");
    result = avformat_find_stream_info(formatContext,0);
    if (result < 0) {
        if (pCallback) {
            pCallback->onErrorAction(THREAD_CHILD,FFMPEG_CAN_NOT_OPEN_URL);
            return;
        }
    }

    //视频时长（单位：微秒: ms, 转换为秒除以1000000）
    duration = formatContext->duration == 0 ? 0 : formatContext->duration / 1000000;

    LOGE("进度 formatContext->duration:%f, duration=%f", formatContext->duration, duration);

    //第三步 根据流信息，流的个数，循环查找，音频流 视频流
    LOGD("第三步 根据流信息，流的个数，循环查找，音频流 视频流");
    //nb_streams  流的个数
    for (int stream_index = 0; stream_index < formatContext->nb_streams; ++stream_index) {
        //第四步 获取媒体流 音频流
        LOGD("第四步 获取媒体流 音频流");
        AVStream *stream = formatContext->streams[stream_index];

        //第五步 从 stream 流中获取解码这段流的参数信息，区分是视频还是音频
        AVCodecParameters *codecParameters = stream->codecpar;

        //第六步 通过流的编解码参数中的编解码 id，获取当前流的解码器
        LOGD("第六步 通过流的编解码参数中的编解码 id，获取当前流的解码器");
        const AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
        //有可能不支持当前编码
        //找不到解码器，重新编译 FFmpeg --enable-librtmp
        if (!codec) {
            pCallback->onErrorAction(THREAD_CHILD,FFMPEG_FIND_DECODER_FAIL);
            return;
        }

        //第七步 通过拿到的解码器，获取解码器上下文
        LOGD("第七步 通过拿到的解码器，获取解码器上下文");
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);

        if (!codecContext) {
            pCallback->onErrorAction(THREAD_CHILD,FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return;
        }

        //第八步 给编辑器上下文 设置参数
        LOGD("第八步 给解码器上下文 设置参数");
        result = avcodec_parameters_to_context(codecContext,codecParameters);
        if (result < 0) {
            pCallback->onErrorAction(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }

        //第九步 打开编辑器
        LOGD("第九步 打开编辑器");
        result = avcodec_open2(codecContext,codec,0);
        if (result) {
            pCallback->onErrorAction(THREAD_CHILD,FFMPEG_OPEN_DECODER_FAIL);
            return;
        }

        //媒体流里面可以拿到时间基
        AVRational baseTime = stream->time_base;

        //第十步 从编码器参数中获取流类型 const_type
        LOGD("第十步 从编码器参数中获取流类型 const_type");
        if (codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {

        } else if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO){

        }


    }



}
