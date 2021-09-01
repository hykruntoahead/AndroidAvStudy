//
// Created by hyk on 21-9-1.
//

#include "AvPlayer.h"

AvPlayer::AvPlayer() {
    //初始化 network支持
    avformat_network_init();
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

    AVFormatContext *avFormatContext = avformat_alloc_context();

    //参数3： 输入文件的封装格式 ，传null表示 自动检测封装格式。 avi / flv
    //参数4： map集合，比如打开网络文件配置的参数，
    // AVDictionary *opts;
    //  av_dict_set(&opts,"timeout","2000000",0);

    int ret = avformat_open_input(&avFormatContext, path, nullptr, nullptr);
    if (ret != 0){
        LOGE(TAG,"打开 %s 失败,错误码为:%d,错误描述:%s",path,ret,av_err2str(ret));
        return;
    }
    LOGD(TAG,"format-打开视频文件成功");
}

