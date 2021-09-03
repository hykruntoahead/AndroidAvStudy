//
// Created by hyk on 21-9-3.
//

#include "VideoChannel.h"

VideoChannel::VideoChannel(int channelId, JavaCallbackHelper *helper, AVCodecContext *avCodecContext,
                           const AVRational &base, int fps):
                           BaseChannel(channelId,helper,avCodecContext,base) ,fps(fps){
}

void *videoDecode_t(void *args){
    auto *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->decode();
    return nullptr;
}

void *videoPlay_t(void *args){
    auto *videoChannel = static_cast<VideoChannel *>(args);
//    videoChannel->play();
    return nullptr;
}

void VideoChannel::play() {
    isPlaying =true;
    //开启工作队列
    setEnable(true);

    //解码
    pthread_create(&videoDecodeTask,0,videoDecode_t,this);
    //播放
    pthread_create(&videoDecodeTask,0,videoPlay_t,this);
}

void VideoChannel::stop() {

}

void VideoChannel::decode() {
    AVPacket *packet = nullptr;
    while (isPlaying){

        int ret = pktQueue.deQueue(packet);
        //停止播放　推出
        if (!isPlaying){
            break;
        }
        //队列没有packet,下论
        if (!ret){
            continue;
        }

        //向解码器发送解码数据
        ret=avcodec_send_packet(avCodecContext,packet);
        releaseAvPacket(packet);
        if (ret < 0){
            break;
        }


        AVFrame  *frame = av_frame_alloc();
        //从解码器取出数据
        ret = avcodec_receive_frame(avCodecContext,frame);
        if (ret == AVERROR(EAGAIN)){//数据获取error,需要重新解码获取
            continue;
        }else if(ret < 0){
            break;
        }
        frameQueue.enQueue(frame);
    }
    releaseAvPacket(packet);
}
