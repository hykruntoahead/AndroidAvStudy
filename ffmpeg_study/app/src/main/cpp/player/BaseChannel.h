//
// Created by hyk on 21-9-3.
//

#ifndef FFMPEG_STUDY_BASECHANNEL_H
#define FFMPEG_STUDY_BASECHANNEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavutil/rational.h>
#ifdef __cplusplus
}
#endif

#include "JavaCallbackHelper.h"
#include "safe_queue.h"

//音视频编解码,播放基类
class BaseChannel{
public:
    int channelId;
    JavaCallbackHelper *helper;
    AVCodecContext *avCodecContext;
    AVRational timeBase;

    SafeQueue<AVPacket *> pktQueue;
    SafeQueue<AVFrame *> frameQueue;
    bool isPlaying = false;

    double clock = 0;
public:
    BaseChannel(int channelId,JavaCallbackHelper *helper,AVCodecContext *avCodecContext,
            AVRational base):channelId(channelId),
                            helper(helper),
                            avCodecContext(avCodecContext),
                            timeBase(base){
            pktQueue.setReleaseHandle(releaseAvPacket);
            frameQueue.setReleaseHandle(releaseAvFrame);
    }

    virtual ~BaseChannel(){
        if(avCodecContext){
            avcodec_close(avCodecContext);
            avcodec_free_context(&avCodecContext);
            avCodecContext = 0;
        }
        pktQueue.clear();
        frameQueue.clear();
    }

    virtual void play() = 0;

    virtual void stop() = 0;

    virtual void decode() = 0;

    void setEnable(bool enable) {
        pktQueue.setEnable(enable);
        frameQueue.setEnable(enable);
    }

    static void releaseAvFrame(AVFrame *&frame) {
        if (frame) {
            av_frame_free(&frame);
            frame = 0;
        }
    }


    static void releaseAvPacket(AVPacket *&packet) {
        if (packet) {
            av_packet_free(&packet);
            packet = 0;
        }
    }
};



#endif //FFMPEG_STUDY_BASECHANNEL_H
