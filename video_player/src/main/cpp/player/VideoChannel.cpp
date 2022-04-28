//
// Created by ASUS on 2022/4/25.
//

#include "include/VideoChannel.h"
#include <unistd.h>

/*
 * 丢包 原始
 */
void dropAVFrame(queue<AVFrame *> &qq){
    if (!qq.empty()) {
        AVFrame *frame = qq.front();
        BaseChannel::releaseAVFrame(&frame);
        qq.pop();
    }
}

/*
 * 丢包 压缩
 */
void dropAVPacket(queue<AVPacket *> &qq){
    if (!qq.empty()) {
        AVPacket *packet = qq.front();
        //这里需要判断删除的是否是关键帧，不能删除关键帧，不然不能编码了
        if (packet->flags != AV_PKT_FLAG_KEY) {
            BaseChannel::releaseAVPacket(&packet);//释放掉
        }
        qq.pop();
    }
}

VideoChannel::VideoChannel(int stream_index, AVCodecContext *pContext, AVRational rational, int fpsValue,
                           JNICallback *jniCallback)
                           : BaseChannel(stream_index,pContext,rational,jniCallback){

    this->fpsValue = fpsValue;
    this->frames.setDeleteVideoFrameCallback(dropAVFrame);
    this->packages.setDeleteVideoFrameCallback(dropAVPacket);
}

VideoChannel::~VideoChannel() {
}

/*
 * 解码线程
 */
void *threadVideoDecode(void *pVoid){
    VideoChannel *videoChannel = static_cast<VideoChannel *>(pVoid);
    videoChannel->video_decode();
    return 0;
}

/*
 * 播放线程
 */
void *threadVideoPlayer(void *pVoid){
    VideoChannel *videoChannel = static_cast<VideoChannel *>(pVoid);
    videoChannel->video_player();
    return 0;
}

/*
 * 1、视频解码 h264 -> YUV
 * 2、视频渲染
 */
void VideoChannel::start() {
    isPlaying = 1;

    //存放为解码数据的队列开始工作了
    packages.setFlag(1);
    //存放解码后数据的队列开始工作
    frames.setFlag(1);

    //1、创建解码线程
    pthread_create(&pid_video_decode,0, threadVideoDecode,this);
    //2、创建播放线程
    pthread_create(&pid_video_player,0,threadVideoPlayer, this);
}

/*
 * 视频解码
 */
void VideoChannel::video_decode() {
    AVPacket *packet = 0;
    while (isPlaying) {
        if (isStop) {
            //线程休眠2秒
//            sleep(2 * 1000 * 1000);
            continue;
        }

        //控制队列大小 避免生产快，消费慢的情况
        if (isPlaying && frames.queueSize() > 100) {
            //线程休眠等待队列中的数据被消费
            av_usleep(10 * 1000); //10s
            continue;
        }

        int result = packages.pop(packet);

        //如果停止播放 跳出循环，出循环，需要释放
        if (!isPlaying) {
            LOGD("isPlaying %d", isPlaying);
            break;
        }

        if (!result) {
            continue;
        }

        //开始取待解码的视频数据包
        result = avcodec_send_packet(pContext,packet);
        if (result) {
            LOGD("result %d",result);
            release(); //TODO ----第二次播放会导致解码失败 目前不知道原因
            break;//失败了
        }

        //释放 packet
        releaseAVPacket(&packet);

        //AVFrame 拿到解码后的原始数据包
        AVFrame *frame = av_frame_alloc();
        result = avcodec_receive_frame(pContext,frame);
        if (result == AVERROR(EAGAIN)) {
            //重新取
            continue;
        } else if (result != 0) {
            LOGD("result %d",result);
            releaseAVFrame(&frame);//内存释放
            break;
        }
        //解码后的视频数据 YUV 放入队列中
        frames.push(frame);
    }

    //出循环，释放
    if (packet) {
        releaseAVPacket(&packet);
    }
}

/*
 * 视频播放，运行在异步线程
 */
void VideoChannel::video_player() {
    //1、原始视频数据 YUV --> rgba
    /*
     * sws_getContext(int srcW, int srcH, enum AVPixelFormat srcFormat,
                                  int dstW, int dstH, enum AVPixelFormat dstFormat,
                                  int flags, SwsFilter *srcFilter,
                                  SwsFilter *dstFilter, const double *param)
     */
    SwsContext *swsContext = sws_getContext(pContext->width,pContext->height,
                                            pContext->pix_fmt,
                                            pContext->width,pContext->height,AV_PIX_FMT_RGBA,
                                            SWS_BILINEAR,NULL,NULL,NULL);
    //2、给 dst_data申请内存
    uint8_t *dst_data[4];
    int dst_linesize[4];
    AVFrame *frame = 0;

    /**
     * pointer[4]: 保存图像通道的地址，如果是 rgb 则前三个指针分别指向R,G,B的内存地址
     * linesize[4]: 保存图像每个通道的内存对齐的步长，即一行的对齐内存的宽度，此值大小等于图像宽度
     * w:要申请内存的图像宽度
     * h: 要申请内存的图像高度
     * pix_fmt: 要申请内存的图像的像素格式
     * align: 用于内存对齐的值
     * 返回值: 所申请的内存空间的总大小，如果是负值，表示申请失败
     */
     int result = av_image_alloc(dst_data,dst_linesize,pContext->width,pContext->height,
                                 AV_PIX_FMT_RGBA,1);
    if (result < 0) {
        printf("Could not allocate source image\n");
        return;
    }

    //3、YUV -> RGBA 格式转换 一帧一帧的转换
    while (isPlaying) {
        if (isStop) {
            //线程休眠
            usleep(2 * 1000 * 1000);
            continue;
        }

        int result = frames.pop(frame);

        //如果停止播放，跳出循环，需要释放
        if (!isPlaying)
            break;

        if (!result)
            continue;

        //真正转换的函数 dst_data 是 rgba 格式的数据
        sws_scale(swsContext, frame->data, frame->linesize, 0, pContext->height, dst_data,
                  dst_linesize);

        //视频向音频时间戳对齐 -》 控制视频播放速度
        //在视频渲染之前，根据 fps 来控制视频帧
        //frame->repeat_pict = 当解码时，这张图片需要延迟多久显示
        double extra_delay = frame->repeat_pict;
        //根据 fps 得到延迟时间
        double base_delay = 1.0 / this->fpsValue;
        //得到当前帧的延迟时间
        double result_delay = extra_delay + base_delay;

        //拿到视频播放的时间基
        this->video_time = frame->best_effort_timestamp * av_q2d(this->base_time);

        //拿到音频播放的时间基
        double_t audioTime = this->audioChannel->audio_time;

        //计算音频和视频的差值
        double av_time_diff = video_time - audioTime;

        LOGE("av_time_diff init audioTime :%f, video：%f", this->audioChannel->audio_time,
             video_time);

        //Notes:
        //video_time < audioTime 视频慢，音频快，需要追赶音频，丢弃掉冗余的视频包，也就是丢帧
        //video_time > audioTime 视频快，音频慢，等待音频
        if (av_time_diff > 0) {
            //通过睡眠的方式灵活等待
            if (av_time_diff > 1) {
                av_usleep((result_delay * 2) * 1000000);
                LOGE("av_time_diff >  1 睡眠:%f", (result_delay * 2) * 1000000);
            } else{//说明相差不大
                av_usleep((av_time_diff + result_delay) * 1000000);
                LOGE("av_time_diff >  1 睡眠:%f", (av_time_diff + result_delay) * 1000000);
            }
        } else if (av_time_diff < 0){
            //视频丢包处理
            this->frames.deleteVideoFrame();
            LOGE("av_time_diff <0  睡眠:%s，丢包:%f ，剩余包: %d", "不睡面", av_time_diff, frames.queueSize());
            continue;
        } else {
            // perfect
        }


    }



}

void VideoChannel::stop() {

}

void VideoChannel::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}

void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {
    this->audioChannel = audioChannel;
}

void VideoChannel::release() {

}

void VideoChannel::restart() {
    isStop = false;
}

