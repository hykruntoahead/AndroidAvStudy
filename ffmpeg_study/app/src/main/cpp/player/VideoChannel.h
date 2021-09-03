//
// Created by hyk on 21-9-3.
//

#ifndef FFMPEG_STUDY_VIDEOCHANNEL_H
#define FFMPEG_STUDY_VIDEOCHANNEL_H
#include "BaseChannel.h"

class VideoChannel: public BaseChannel {
private:
    int fps;
    pthread_t videoDecodeTask,videoPlayTask;
    bool isPlaying;

public:
    VideoChannel(int channel,JavaCallbackHelper *helper,
            AVCodecContext *avCodecContext,const AVRational &base,int fps);

public:
    virtual void play();

    virtual void stop();

    virtual void decode();

};


#endif //FFMPEG_STUDY_VIDEOCHANNEL_H
