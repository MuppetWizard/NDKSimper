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
    //可以动态获取， 如何计算 ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓

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

        //重采样
        /*
         * @param out_buffers            输出缓冲区，当PCM数据为Packed包装格式时，只有out[0]会填充有数据。
         * @param dst_nb_samples         每个通道可存储输出PCM数据的sample数量。
         * @param pcmFrame->data         输入缓冲区，当PCM数据为Packed包装格式时，只有in[0]需要填充有数据。
         * @param pcmFrame->nb_samples   输入PCM数据中每个通道可用的sample数量。
         *
         * @return  返回每个通道输出的sample数量，发生错误的时候返回负数。
         */
        result = swr_convert(swr_context,&out_buffers,dst_nb_samples,(const uint8_t **) pcmFrame->data,
                             pcmFrame->nb_samples); // 返回每个通道输出的sample数量，发生错误的时候返回负数
        if (result < 0) {
            fprintf(stderr,"Error while converting\n");
        }

        pcm_data_size = result * out_sample_size * out_channel;

        //用于音视频同步
        audio_time = pcmFrame->best_effort_timestamp * av_q2d(this->base_time);

        if (javaCallHelper) {
            javaCallHelper->onProgress(THREAD_CHILD,audio_time);
        }
        break;
    }
    //渲染完成，释放资源
    releaseAVFrame(&pcmFrame);

    return pcm_data_size;
}

/*
 * 创建播放音频的回调函数
 */
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void * context){
    AudioChannel *audioChannel = static_cast<AudioChannel *>(context);
    //获取 PCM 音频裸流
    int pcmSize = audioChannel->getPCM();
    if (!pcmSize) return;
    (*bq)->Enqueue(bq, audioChannel->out_buffers,pcmSize);
}

/*
 * 音频播放 直接使用 OpenLS ES 渲染 PCM 数据
 */
void AudioChannel::audio_player() {
    // 1、创建引擎 并获取引擎接口
    // 1.1、创建引擎对象: SLObjectItf engineObject
    SLresult result = slCreateEngine(&engineObject,0,NULL,0,NULL,NULL);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    // 1.2 初始化引擎
    result = (*engineObject)->Realize(engineObject,SL_BOOLEAN_FALSE);
    if (SL_BOOLEAN_FALSE != result) {
        return;
    }

    // 1.3、获取引擎接口 SLEngineItf engineInterface
    result = (*engineObject)->GetInterface(engineObject,SL_IID_ENGINE,&engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    // 2、设置混音器
    // 2.1 创建混音器 SLObjectItf outputMixObject
    result = (*engineInterface)->CreateOutputMix(engineInterface,&outputMixObject,0,0,0);
    if (result != SL_RESULT_SUCCESS) {
        return;
    }

    // 2.2 初始化 混音器
    result = (*outputMixObject)->Realize(outputMixObject,SL_BOOLEAN_FALSE);
    if (SL_BOOLEAN_FALSE != result) {
        return;
    }

    //  不启用混响可以不用获取混音器接口
    //  获得混音器接口
    //  result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
    //                                         &outputMixEnvironmentalReverb);
    //  if (SL_RESULT_SUCCESS == result) {
    //  设置混响 ： 默认。
    //  SL_I3DL2_ENVIRONMENT_PRESET_ROOM: 室内
    //  SL_I3DL2_ENVIRONMENT_PRESET_AUDITORIUM : 礼堂 等
    //  const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
    //  (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
    //       outputMixEnvironmentalReverb, &settings);
    //  }

    // 3、创建播放器
    // 3.1 配置输入声音信息
    // 创建buffer缓冲类型的队列 2个队列
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};

    // pcm音频数据格斯
    // SL_DATAFORMAT_PCM：数据格式为pcm格式
    // 2：双声道
    // SL_SAMPLINGRATE_44_1：采样率为44100（44.1赫兹 应用最广的，兼容性最好的）
    // SL_PCMSAMPLEFORMAT_FIXED_16：采样格式为16bit （16位）(2个字节)
    // SL_PCMSAMPLEFORMAT_FIXED_16：数据大小为16bit （16位）（2个字节）
    // SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT：左右声道（双声道）  （双声道 立体声的效果）
    // SL_BYTEORDER_LITTLEENDIAN：小端模式
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM,2,SL_SAMPLINGRATE_44_1,
                                   SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                                   SL_BYTEORDER_LITTLEENDIAN};

    //数据源 将上述配置信息放到这个数据源中
    SLDataSource audioSrc = {&loc_bufq,&format_pcm};

    // 3.2、配置音轨(输出)
    //设置混音器
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX,outputMixObject};
    SLDataSink audio_sink = {&loc_outmix,NULL};

    // 需要的接口，操作队列的接口
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    // 3.3 创建播放器
    result = (*engineInterface)->CreateAudioPlayer(engineInterface,&bqPlayerObjct,&audioSrc,
                                                   &audio_sink,1,ids,req);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    // 3.4 初始化播放器 SLObjectItf bqPlayerObject
    result = (*bqPlayerObjct)->Realize(bqPlayerObjct,SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    // 3.5 获取播放器接口 SLPlayItf bqPlayerPlay
    result = (*bqPlayerObjct)->GetInterface(bqPlayerObjct,SL_IID_PLAY,&bqPlayerPlay);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    // 4、设置播放器回调函数
    // 4.1 获取播放器队列接口: SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue
    (*bqPlayerObjct)->GetInterface(bqPlayerObjct,SL_IID_BUFFERQUEUE,&bqPlayerBufferQueue);

    // 4.2 设置回调 void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);

    // 5、设置播放状态
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay,SL_PLAYSTATE_PLAYING);

    // 6 手动激活回调函数
    bqPlayerCallback(bqPlayerBufferQueue, this);
}

/*
 * 停止播放
 */
void AudioChannel::stop() {
    isStop = true;
    if (javaCallHelper) {
        javaCallHelper = 0;
    }

}


/*
 * 开始播放
 */
void AudioChannel::start() {
    //设置正在播放标签
    isPlaying = true;
    //存放 解码的队列开始工作
    packages.setFlag(1);
    //存放解码后的队列开始工作了
    frames.setFlag(1);

    //1、解码线程
    pthread_create(&pid_audio_decode,0, thread_audio_decade, this);
    //2、渲染线程
    pthread_create(&pid_audio_player,0,thread_audio_play, this);
}

/*
 * 释放操作
 */
void AudioChannel::release() {
    LOGE("AudioChannel ：%s", "执行了销毁");
    // 5 设置暂停播放标签
    isPlaying = false;
    isStop = true;

    // 1、设置停止状态
    if (bqPlayerPlay) {
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay,SL_PLAYSTATE_STOPPED);
        bqPlayerPlay = 0;
    }
    // 2、销毁播放器
    if (bqPlayerObjct) {
        (*bqPlayerObjct)->Destroy(bqPlayerObjct);
        bqPlayerObjct = 0;
        bqPlayerBufferQueue = 0;
    }
    // 3、销毁混音器
    if (outputMixObject) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = 0;
    }
    // 4、销毁引擎
    if (engineObject) {
        (*engineObject)->Destroy(engineObject);
        engineObject = 0;
    }

    if (frames.queueSize() > 0) {
        frames.clearQueue();
    }

    if (packages.queueSize() > 0) {
        packages.clearQueue();
    }

    if (out_buffers) {
        free(out_buffers);
        out_buffers = 0;
    }
}

void AudioChannel::reStart() {
    isStop = false;
}



