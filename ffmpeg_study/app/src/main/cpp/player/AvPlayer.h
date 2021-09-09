//
// Created by hyk on 21-9-1.
//

#ifndef FFMPEG_STUDY_AVPLAYER_H
#define FFMPEG_STUDY_AVPLAYER_H

#include <pthread.h>
#include "../util/LogUtil.h"
#include "JavaCallbackHelper.h"
#include "VideoChannel.h"
#include "AudioChannel.h"
//#include <android/native_window.h>
extern "C" {
#include <libavformat/avformat.h>
}

class AvPlayer {
    const char *TAG = "NativeAvPlayer";

    friend void *prepare_t(void *args);

    friend void *start_t(void *args);

public:
    AvPlayer(JavaCallbackHelper *helper);

public:
    void setDataSource(const char *path_);
    void prepare();
    void start();
    void setWindow(ANativeWindow *nativeWindow);

private:
    char *path;
    pthread_t prepareTd;
    JavaCallbackHelper *helper;
    int64_t duration;
    VideoChannel *videoChannel;
    AudioChannel *audioChannel;
    pthread_t startTask;
    bool isPlaying;
    AVFormatContext *avFormatContext;

    ANativeWindow *window = 0;

private:
    void _prepare_t();

    void _start_t();
};


#endif //FFMPEG_STUDY_AVPLAYER_H
