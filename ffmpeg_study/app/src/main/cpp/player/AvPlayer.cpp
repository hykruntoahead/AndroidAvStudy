//
// Created by hyk on 21-9-1.
//

#include "AvPlayer.h"

AvPlayer::AvPlayer(JavaCallbackHelper *helper) : helper(helper){
    //初始化 network支持
    avformat_network_init();
    videoChannel = nullptr;
}

void AvPlayer::setDataSource(const char *path_) {
    //    path  = static_cast< char *>(malloc(strlen(path_) + 1));
    //    memset((void *) path, 0, strlen(path) + 1);
    //   memcpy(path,path_,strlen(path_));
    path = new char[strlen(path_)+1];
    strcpy(path,path_);
}


void* prepare_t(void *args){
    auto* avPlayer = static_cast<AvPlayer *>(args);
    avPlayer->_prepare_t();

    return nullptr;
}

void AvPlayer::prepare() {
    /**
     * 因为音视频解码会耗时,所以放在子线程进行.
     * 这里使用 POSIX线程库 pthread
     * 参数解释:
     * 第一个参数为指向线程 标识符的 指针。
     * 第二个参数用来设置线程属性。
     * 第三个参数是线程运行函数的起始地址。
     * 最后一个参数是运行函数的参数。
     * 这里传入avplayer实例 this
     */
    pthread_create(&prepareTd, nullptr,prepare_t, this);
}

void AvPlayer::_prepare_t() {
    LOGD(TAG,"运行子线程,内存地址为:%ld",prepareTd);
    avFormatContext = avformat_alloc_context();

    //参数3： 输入文件的封装格式 ，传null表示 自动检测封装格式。 avi / flv
    //参数4： map集合，比如打开网络文件配置的参数，
    // AVDictionary *opts;
    //  av_dict_set(&opts,"timeout","2000000",0);

    /**
     * 1.打开媒体文件
     */
    int ret = avformat_open_input(&avFormatContext, path, nullptr, nullptr);
    if (ret != 0){
        LOGE(TAG,"打开 %s 失败,错误码为:%d,错误描述:%s",path,ret,av_err2str(ret));
        helper->onError(FFMPEG_CAN_NOT_OPEN_URL,THREAD_CHILD);
        return;
    }

    /**
     * 2.查找媒体流
     */
     ret = avformat_find_stream_info(avFormatContext, nullptr);
     if (ret < 0){
         LOGE("查找媒体流 %s 失败，返回:%d 错误描述:%s", path, ret, av_err2str(ret));
         helper ->onError(FFMPEG_CAN_NOT_FIND_STREAMS,THREAD_CHILD);
         return;
     }

     //得到视频时长，单位是s
     duration = avFormatContext->duration / AV_TIME_BASE;

    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        //音视频流　结构体
        AVStream *avStream = avFormatContext->streams[i];
        //解码信息　参数
        AVCodecParameters  *parameters = avStream->codecpar;
        //解码器
        AVCodec *dec = avcodec_find_decoder(parameters->codec_id);
        if (!dec){
            helper->onError(FFMPEG_FIND_DECODER_FAIL,THREAD_CHILD);
            return;
        }

        //获取解码上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(dec);

        //把解码信息赋值给解码上下文中各成员
        if (avcodec_parameters_to_context(codecContext,parameters) < 0){
            helper->onError(FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL,THREAD_CHILD);
            return;
        }

        //打开解码器
        if (avcodec_open2(codecContext,dec, nullptr) !=0){
            helper->onError(FFMPEG_OPEN_DECODER_FAIL,THREAD_CHILD);
            return;
        }

        //音频
        if (parameters->codec_type == AVMEDIA_TYPE_AUDIO){
            audioChannel = new AudioChannel(i,helper,codecContext,avStream->time_base);
            //视频
        } else if (parameters->codec_type == AVMEDIA_TYPE_VIDEO){
            //获得帧率
            int fps = av_q2d(avStream->avg_frame_rate);
            //创建videoChannel，用来之后的解码＆播放
            videoChannel = new VideoChannel(i,helper,codecContext,avStream->time_base,fps);
        }
    }

    //如果媒体文件中没有视频 & 没有音频
    if (!videoChannel && !audioChannel){
        helper->onError(FFMPEG_NOMEDIA,THREAD_CHILD);
        return;
    }
    //call java　已准备好，可以播放了
    helper->onPrepared(THREAD_CHILD);
}

void *start_t(void *args) {
    auto *player = static_cast<AvPlayer *>(args);
    player->_start_t();
    return nullptr;
}

void AvPlayer::start() {
    //1.读取媒体源数据
    //2.根据数据类型放入　Audio/VideoChannel的队列中
    LOGD(TAG,"start called");
    isPlaying = true;
    if (videoChannel){
        videoChannel->play();
    }

    if(audioChannel){
        audioChannel->play();
    }

    pthread_create(&startTask,nullptr,start_t,this);
}

void AvPlayer::_start_t() {
    int ret;
    while(isPlaying){
        //没解码前的数据包
        AVPacket *packet = av_packet_alloc();
        //Return the next frame of a stream.
        ret = av_read_frame(avFormatContext,packet);
        if(ret == 0){
            //为video类型数据包
            if(videoChannel && packet-> stream_index == videoChannel->channelId){
                videoChannel->pktQueue.enQueue(packet);
            } else if(audioChannel && packet->stream_index == audioChannel->channelId){
                LOGD(TAG,"enQueue audioChannel packet")
                audioChannel->pktQueue.enQueue(packet);
            } else{
                av_packet_free(&packet);
            }
        }else if(ret == AVERROR_EOF){
            //读取完毕，不一定播放完毕
            if(videoChannel->pktQueue.empty() &&
            videoChannel->frameQueue.empty()){
                break;
            }
        } else{
            break;
        }
    }
    isPlaying = false;
    audioChannel->stop();
    videoChannel->stop();
}

void AvPlayer::setWindow(ANativeWindow *nativeWindow) {
    this->window = nativeWindow;
    if(videoChannel){
        LOGD(TAG,"videoChannel setWindow called")
        videoChannel->setWindow(nativeWindow);
    }
}





