//
// Created by ASUS on 2022/4/22.
//

#include "include/MPlayer.h"
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
            audioChannel = new AudioChannel(stream_index,codecContext,baseTime,pCallback);
        } else if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO){
            //获取视频 fps
            //平均帧率 = 时间基
            AVRational frame_rate = stream->avg_frame_rate;
            int fps_value = av_q2d(frame_rate);
            videoChannel = new VideoChannel(stream_index, codecContext, baseTime, fps_value, pCallback);
            videoChannel->setRenderCallback(renderCallback);
        }
    }//end for


    //第十一步 如果流中没有音频和视频数据
    LOGD("第十一步 如果流中没有音视频数据");
        if (!audioChannel && !videoChannel){
            pCallback->onErrorAction(THREAD_CHILD,FFMPEG_NOMEDIA);
            return;
        }

    //第十二步 要么有视频、要么有音频，要么音视频都有
    LOGD("第十二步 要么有视频、要么有音频，要么音视频都有");
    if (this->pCallback) {
        pCallback->onPrepared(THREAD_CHILD);
    }
}

void MPlayer::start() {
    //声明是否播放标记
    isPlaying = 1;
    if (videoChannel) {
        videoChannel->setAudioChannel(audioChannel);
        videoChannel->start();
    }

    if (audioChannel) {
        audioChannel->start();
    }

    //定义一个播放线程
    pthread_create(&pid_start, 0, customTaskStartThread, this);


}

void MPlayer::start_() {
    //循环读音视频包
    while (isPlaying) {
        if (!formatContext) {
            LOGD("FormatContext: %d", "formatConstext");
            return;
        }

        if (isStop) {
            //usleep(2 * 1000 * 1000);
            continue;
        }
        LOGD("start_");

        //内存泄漏点 fix: 控制队列大小
        if (videoChannel && videoChannel->packages.queueSize() > 100) {
            //休眠 等待队列中的数据被消费
            av_usleep(10 * 1000);
            continue;
        }

        //内存泄漏点 fix: 控制队列大小
        if (audioChannel && audioChannel->packages.queueSize() > 100) {
            //休眠 等待队列中的数据被消费
            av_usleep(10 * 1000);
            continue;
        }

        //AVPacket 可能是音频或者视频，没有解码的数据包
        AVPacket *packet = av_packet_alloc();

        //这一行执行完毕， packet 就有音视频数据了
        int result = av_read_frame(formatContext,packet);
        if (!result) {
            if (videoChannel && videoChannel->stream_index == packet->stream_index) {//视频包
                LOGE("stream_index 视频 %s", "push");
                //未解码的 视频数据包 加入队列
                videoChannel->packages.push(packet);
            } else if (audioChannel && audioChannel->stream_index == packet->stream_index){//音频包
                LOGE("stream_index 音频 %s", "push");
                //将语音包加入到队列中，以供解码使用
                audioChannel->packages.push(packet);
            }
        } else if (result == AVERROR_EOF) {
            LOGE("stream_index 拆包完成 %s", "读取完成了");
            isPlaying = 0;
            stop();
            release();
            break;
        } else {
            LOGE("stream_index 拆包 %s", "读取失败");
            break;
        }
    } //end while
    //最后释放工作
    isPlaying = 0;
    isStop = false;
    videoChannel->stop();
    audioChannel->stop();

}

void MPlayer::stop() {
    isStop = true;
    if (videoChannel) videoChannel->stop();
    if (audioChannel) audioChannel->stop();
    if (videoChannel) videoChannel->javaCallHelper = 0;
    if (audioChannel) audioChannel->javaCallHelper = 0;
}

/*
 * 控制播放
 */
void MPlayer::seek(int i) {
    //必须在 0 - duration 范围之间
    if (i < 0 || i >= duration) return;
    if (!audioChannel && !videoChannel) return;
    if (!formatContext) return;

    isSeek = 1;
    // ----
    pthread_mutex_lock(&seekMutex);
    //单位 微秒
    int64_t seek = i * 1000000;
    //seek到请求的时间 之前最近的关键帧
    //只有从关键帧开始才能解码出完整图片
    av_seek_frame(formatContext,-1,seek,AVSEEK_FLAG_BACKWARD);
//    avformat_seek_file(formatContext, -1, INT64_MIN, seek, INT64_MAX, 0);

    //音频与视频队列中的数据 是不是可以丢掉了？
    if (audioChannel) {
        // 可以清空缓存
        audioChannel->clear();
        //自动队列
    }

    if (videoChannel) {
        videoChannel->clear();
    }
    pthread_mutex_unlock(&seekMutex);
    isSeek = 0;
}

void MPlayer::restart() {
    isStop = false;
    if (videoChannel) videoChannel->restart();
    if (audioChannel) audioChannel->reStart();

}

void MPlayer::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}

void MPlayer::release() {
    LOGD("YKPlayer ：%s", "执行了销毁");
    isPlaying = false;
    stop();
    if (videoChannel) videoChannel->release();
    if (audioChannel) audioChannel->release();
    if (codecContext) {
        avcodec_free_context(&codecContext);
        codecContext = 0;
    }
    if (formatContext) {
        avformat_free_context(formatContext);
        formatContext = 0;
    }

    duration = 0;
}
