//
// Created by hyk on 21-9-9.
//

#ifndef FFMPEG_STUDY_AUDIOCHANNEL_H
#define FFMPEG_STUDY_AUDIOCHANNEL_H

#include "BaseChannel.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "../util/LogUtil.h"

extern "C"{
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
};

class AudioChannel : public BaseChannel{
    const char *TAG = "AudioChannel";

    friend void *audioPlay_t(void *args);
    friend void bqPlayerCallback(SLAndroidSimpleBufferQueueItf queueItf, void *pContext);

public:
    AudioChannel(int channelId,JavaCallbackHelper *helper,AVCodecContext *avCodecContext,
                const AVRational &base);
    ~AudioChannel();

public:
    virtual void play();

    virtual void stop();

    virtual void decode();

private:
    void _play();
    int _getData();
    void _releaseOpenSL();

private:
    pthread_t audioDecodeTask, audioPlayTask;
    SwrContext *swrContext = 0;
    uint8_t *buffer;
    int bufferCount;
    int out_channels;
    int out_sampleSize;

    SLObjectItf engineObject = NULL;
    SLEngineItf engineInterface = NULL;
    SLObjectItf outputMixObject = NULL;
    SLObjectItf bqPlayerObject = NULL;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = NULL;
    SLPlayItf bqPlayerInterface = NULL;
};


#endif //FFMPEG_STUDY_AUDIOCHANNEL_H
