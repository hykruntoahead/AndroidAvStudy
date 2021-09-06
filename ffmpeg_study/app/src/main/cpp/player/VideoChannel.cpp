//
// Created by hyk on 21-9-3.
//

#include <LogUtil.h>
#include "VideoChannel.h"

VideoChannel::VideoChannel(int channelId, JavaCallbackHelper *helper, AVCodecContext *avCodecContext,
                           const AVRational &base, int fps):
                           BaseChannel(channelId,helper,avCodecContext,base) ,fps(fps){
    pthread_mutex_init(&surfaceMutex, nullptr);
}


VideoChannel::~VideoChannel() {
    pthread_mutex_destroy(&surfaceMutex);
}


void *videoDecode_t(void *args){
    auto *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->decode();
    return nullptr;
}

void *videoPlay_t(void *args){
    auto *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->_play();
    return nullptr;
}

void VideoChannel::play() {
    isPlaying =true;
    //开启工作队列
    setEnable(true);

    //解码
    pthread_create(&videoDecodeTask,nullptr,videoDecode_t,this);
    //播放
    pthread_create(&videoPlayTask,nullptr,videoPlay_t,this);
}



void VideoChannel::decode() {
    AVPacket *packet = nullptr;
    while (isPlaying){

        int ret = pktQueue.deQueue(packet);
        //停止播放　推出
        if (!isPlaying){
            break;
        }
        //队列没有packet,下一轮
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

void VideoChannel::_play() {
    //缩放，格式转换
    /**参数分别为:
    转换前宽高与格式，
    转换后宽高与格式，
    转换使用的算法，
    输入/输出图像滤波器，
    特定缩放算法需要的参数
     **/
     //将avFrame原图像格式(YUV)转换为RGBA
    SwsContext *swsContext = sws_getContext(
            avCodecContext->width,
            avCodecContext->height,
            avCodecContext->pix_fmt,
            avCodecContext->width,
            avCodecContext->height,
            AV_PIX_FMT_RGBA,
            SWS_FAST_BILINEAR,
            nullptr,nullptr, nullptr);

    //转换后的数据与每行数据字节数
    uint8_t *data[4];
    int linesize[4];
    //根据格式申请内存
    av_image_alloc(data,linesize,avCodecContext->width,
            avCodecContext->height,AV_PIX_FMT_RGBA,1);

    AVFrame *frame = nullptr;//解码后待转换的结构体
    int  ret;
    while (isPlaying){
        //阻塞方法
        ret = frameQueue.deQueue(frame);
        //用户停止播放
        if(!isPlaying){
            break;
        }
        if(!ret){
            continue;
        }

        // 2、指针数据，比如RGBA，
        // 每一个维度的数据就是一个指针，那么RGBA需要4个指针，
        // 所以就是4个元素的数组，数组的元素就是指针，指针数据
        // 3、每一行数据他的数据个数
        // 4、 offset
        // 5、 要转化图像的高
        //目标图像data,目标图像每一行数据
        sws_scale(swsContext,frame->data,frame->linesize,0,
                frame->height,data,linesize);
        // 绘制
        onDraw(data,linesize,avCodecContext->width,avCodecContext->height);
    }
    av_free(&data[0]);
    isPlaying = false;
    releaseAvFrame(frame);
    sws_freeContext(swsContext);

}

void VideoChannel::onDraw(uint8_t **data, int *linesize, int width, int height) {
    pthread_mutex_lock(&surfaceMutex);
    if(!window){
        pthread_mutex_unlock(&surfaceMutex);
        return;
    }
    //设置ANativeWindow属性　宽，高　颜色格式
    ANativeWindow_setBuffersGeometry(window,width,height,WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer buffer;

    //lock获得ANativeWindow需要显示的数据缓存
    if(ANativeWindow_lock(window,&buffer,nullptr) != 0){
        ANativeWindow_release(window);
        window = nullptr;
        pthread_mutex_unlock(&surfaceMutex);
        return;
    }

    //把视频数据刷到buffer
    uint8_t *dstData = static_cast<uint8_t *>(buffer.bits);
    //window一行需要多少个数据*4(rgba)
    int dstSize = buffer.stride * 4;

    //视频图像的rgba数据
    uint8_t *srcData = data[0];
    int srcSize = linesize[0];

    LOGD(TAG,"onDraw srcSize=%d,dstSize=%d",srcSize,dstSize);
    //按行拷贝
    for (int i=0;i<buffer.height;++i){
        memcpy(dstData+i*dstSize,srcData+i*srcSize,srcSize);
    }

    ANativeWindow_unlockAndPost(window);

    pthread_mutex_unlock(&surfaceMutex);
}


void VideoChannel::stop() {

}

void VideoChannel::setWindow(ANativeWindow *nativeWindow) {
    pthread_mutex_lock(&surfaceMutex);
    if(this->window){
        ANativeWindow_release(this->window);
    }
    this->window = nativeWindow;
    pthread_mutex_unlock(&surfaceMutex);
}

