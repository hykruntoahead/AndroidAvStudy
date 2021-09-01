//
// Created by hyk on 21-9-1.
//

#ifndef FFMPEG_STUDY_AVPLAYER_H
#define FFMPEG_STUDY_AVPLAYER_H
#include <pthread.h>
#include "../util/LogUtil.h"
#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/version.h>
#include <libavcodec/avcodec.h>
#include <libavformat/version.h>
#include <libavutil/version.h>
#include <libavfilter/version.h>
#include <libswresample/version.h>
#include <libswscale/version.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#ifdef __cplusplus
}
#endif

class AvPlayer {
    const char * TAG = "NativeAvPlayer";

    friend void* prepare_t(void *args);

    private:
        char* path;
        pthread_t prepareTd;

    public:
        AvPlayer();
        void setDataSource(const char* path_);
        void prepare();
        void _prepare_t();
};


#endif //FFMPEG_STUDY_AVPLAYER_H
