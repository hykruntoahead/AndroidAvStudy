//
// Created by hyk on 21-9-3.
//

#ifndef FFMPEG_STUDY_VIDEOCHANNEL_H
#define FFMPEG_STUDY_VIDEOCHANNEL_H

#include "BaseChannel.h"
#include <android/native_window_jni.h>
#include <android/native_window.h>

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/rational.h>
#include <libavutil/imgutils.h>
};

class VideoChannel : public BaseChannel {

    const char * TAG = "VideoChannel";

    friend void *videoPlay_t(void *args);

private:
    int fps;
    pthread_t videoDecodeTask, videoPlayTask;
    bool isPlaying;
    pthread_mutex_t surfaceMutex;
    ANativeWindow *window = 0;

    void _play();

    void onDraw(uint8_t *data[4], int linesize[4], int width, int height);

public:
    VideoChannel(int channel, JavaCallbackHelper *helper,
                 AVCodecContext *avCodecContext, const AVRational &base, int fps);


    ~VideoChannel();

public:
    virtual void play();

    virtual void stop();

    virtual void decode();

    void setWindow(ANativeWindow *nativeWindow);


};


#endif //FFMPEG_STUDY_VIDEOCHANNEL_H
