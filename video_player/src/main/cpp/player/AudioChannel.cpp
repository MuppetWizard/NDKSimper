//
// Created by ASUS on 2022/4/25.
//

#include "include/AudioChannel.h"

AudioChannel::AudioChannel(int stream_index, AVCodecContext *pContext, AVRational avRational,
                           JNICallback *jniCallback) : BaseChannel(stream_index, pContext,avRational,jniCallback){

    //初始化缓冲区 out_buffer
    //如果定义缓冲区
    //根据音频类型 44100 和 16bit 和双声道
    //可以写死 44100 * 2 * 2
    //可以动态获取， 如何计算 ↓↓↓↓↓↓↓↓

    //获取双声道
    out_channel = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    //获取采样 size
    out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    //计算缓冲大小
    out_buffers_size = out_sample_size * out_sample_rate * out_channel;

    //分缓冲内存
    out_buffers = static_cast<uint8_t *>(malloc(out_buffers_size));
    //void *memset(void *str, int c, size_t n)  复制字符 c（一个无符号字符）到参数 str 所指向的字符串的前 n 个字符。
    memset(out_buffers,0,out_buffers_size);
    //根据通道布局，音频数据格式，采样频率，返回分配的转换上下文 SwrContext 指针
    swr_context = swr_alloc_set_opts(0,AV_CH_LAYOUT_STEREO,AV_SAMPLE_FMT_S16,out_sample_rate,
                                     pContext->channel_layout,pContext->sample_fmt,
                                     pContext->sample_rate,0,0);
    //初始化上下文
    swr_init(swr_context);

    LOGE("AudioChannel---");
}

AudioChannel::~AudioChannel() {
}

/*
 * 解码线程
 */
void *thread_audio_decade(void *pVoid){
    AudioChannel *audioChannel = static_cast<AudioChannel *>(pVoid);
    audioChannel->audio_decode(); //真正执行音频编码的函数
    return 0;
}

/*
 * 播放线程
 */
void *thread_audio_play(void *pVoid){
    AudioChannel *audioChannel = static_cast<AudioChannel *>(pVoid);
    audioChannel->audio_player(); //真正执行播放的函数
    return 0;
}

/*
 * 音频解码
 */
void AudioChannel::audio_decode() {
    //待解码的packet
    AVPacket *avPacket = 0;
    //只要正在播放，就循环取数据
    while (isPlaying) {
        if (isStop) {
            //线程休眠
//            av_usleep(2*1000);
            continue;
        }

        //控制队列大小 当生产快，消费慢时会造成因队列数据过多而引起OOM
        if (isPlaying && frames.queueSize() > 100) {
            //线程休眠 10s
            av_usleep(10 * 1000);
            continue;
        }

        // 可以正常取出
        int result = packages.pop(avPacket);
        //条件判断可以继续
        if (!result) continue;
        if (!isPlaying) break;

        //待解码的数据发送到解码器中
        result = avcodec_send_packet(pContext,avPacket);// return 0 on success. otherwise negative error code

        if (result) break;// 发送到解码器失败

        //发送成功，释放 packet
        releaseAVPacket(&avPacket);

        //拿到解码后的原始数据包
        AVFrame *avFrame = av_frame_alloc();
        //将原始数据发送到 avFrame 内存中去
        result = avcodec_receive_frame(pContext,avFrame);

        if (result == AVERROR(EAGAIN)){
            continue; //获取失败，继续下次任务
        } else if (result != 0) {
            releaseAVFrame(&avFrame); //释放申请内容
            break;
        }

        //将获取到的原始数据放入队列中，也就是解码后的数据
        frames.push(avFrame);
    }

    if (avPacket) {
        releaseAVPacket(&avPacket);
    }
}

/*
 * 获取PCM数据
 */
int AudioChannel::getPCM() {
    //定义 PCM 数据的大小
    int pcm_data_size = 0;

    //原始数据包装类
    AVFrame *pcmFrame = 0;
    //循环取出
    while (isPlaying) {
        if (isStop) {
            continue;
        }

        int result = frames.pop(pcmFrame);
        if (!isPlaying) break;
        if (!result) continue;

        //PCM处理逻辑
        pcmFrame->data;
        // 音频播放器的数据格式是我们在下面定义的（16位 双声道 ....）
        // 而原始数据（是待播放的音频PCM数据）
        // 所以，上面的两条，无法统一，一个是(自己定义的16位 双声道 ..) 一个是原始数据，为了解决上面的问题，就需要重采样
        //开始重采样
        int dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_context,pcmFrame->sample_rate) +
                pcmFrame->nb_samples,out_sample_rate,
                pcmFrame->sample_rate,AV_ROUND_UP);


    }

    return 0;
}

