//
// Created by hyk on 21-9-9.
//

#include "AudioChannel.h"

AudioChannel::AudioChannel(int channelId, JavaCallbackHelper *helper,
                           AVCodecContext *avCodecContext, const AVRational &base)
        : BaseChannel(channelId, helper, avCodecContext, base) {
    //爽声道
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    //采样位　16,2字节
    out_sampleSize = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    //采样率
    int out_sampleRate = 44100;

    //计算转换后数据的最大字节数
    bufferCount = out_sampleRate * out_sampleSize * out_channels;
    buffer = static_cast<uint8_t *>(malloc(bufferCount));

}

AudioChannel::~AudioChannel() {
    free(buffer);
    buffer = nullptr;
}


void *audioDecode_t(void *args) {
    auto *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->decode();
    return nullptr;
}

void *audioPlay_t(void *args) {
    auto *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->_play();
    return nullptr;
}

void AudioChannel::play() {
    //分配SwrContext并设置/重置公共参数
    swrContext = swr_alloc_set_opts(nullptr, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
                                    avCodecContext->channel_layout, avCodecContext->sample_fmt,
                                    avCodecContext->sample_rate,
                                    0, nullptr);

    swr_init(swrContext);

    isPlaying = true;
    setEnable(true);

    //解码
    pthread_create(&audioDecodeTask, nullptr, audioDecode_t, this);
    //播放
    pthread_create(&audioPlayTask, nullptr, audioPlay_t, this);
}

void AudioChannel::stop() {
    isPlaying = false;
    helper = nullptr;
    setEnable(false);
    pthread_join(audioDecodeTask, nullptr);
    pthread_join(audioPlayTask, nullptr);

    _releaseOpenSL();

    if (swrContext){
        swr_free(&swrContext);
        swrContext = nullptr;
    }
}

//AvPacket -> AvFrame
void AudioChannel::decode() {
    AVPacket *packet = nullptr;
    while (isPlaying) {
        int ret = pktQueue.deQueue(packet);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }

        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAvPacket(packet);

        if (ret < 0) {
            break;
        }

        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            break;
        }
        while (frameQueue.size() > 100 && isPlaying){
            av_usleep(1000*10);
        }
        frameQueue.enQueue(frame);
        LOGD(TAG, "AudioChannel::decode enQueue frame")
    }
}

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf queueItf, void *pContext) {
    auto *audioChannel = static_cast<AudioChannel *>(pContext);

    int dataSize = audioChannel->_getData();
    if (dataSize > 0) {
        //加入队列并播放
        (*queueItf)->Enqueue(queueItf, audioChannel->buffer, dataSize);
    }
    LOGD(audioChannel->TAG, "bqPlayerCallback:dataSize=%d", dataSize);
}

void AudioChannel::_play() {
    LOGD(TAG, "audioChannel play 创建引擎");
    /**
     * 1.创建引擎
     */
    SLresult result;
    result = slCreateEngine(&engineObject, 0, nullptr,
                            0, nullptr, nullptr);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    LOGD(TAG, "audioChannel play engineObject　初始化");
    //初始化
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    //获取引擎接口 engineInterface
    result = (*engineObject)->GetInterface(engineObject,
                                           SL_IID_ENGINE,
                                           &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    LOGD(TAG, "audioChannel  engineInterface　获取引擎接口");
    /**
     * 2.创建混音器
     */
    //通过引擎接口创建混音器
    result = (*engineInterface)->CreateOutputMix(engineInterface,
                                                 &outputMixObject, 0, nullptr, nullptr);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    LOGD(TAG, "audioChannel  outputMixObject　通过引擎接口创建混音器");

    //初始化混音器outputMixObject
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    LOGD(TAG, "audioChannel  outputMixObject　初始化混音器");
    /**
     * 3.创建播放器
     */
    SLDataLocator_AndroidSimpleBufferQueue androidQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                           2};

    //pcm数据格式：pcm,声道数，采样率，采样位，容器大小，通道掩码(双声道)，字节序(小端)
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1, SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};
    //数据源(数据获取器＋格式)　从什么地方获取播放数据
    SLDataSource slDataSource = {&androidQueue, &pcm};

    //设置混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSink = {&outputMix, nullptr};

    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    //播放器相当于对混音器进行了一层包装， 提供了额外的如：开始，停止等方法。
    //混音器来播放声音
    (*engineInterface)->CreateAudioPlayer(engineInterface, &bqPlayerObject, &slDataSource,
                                          &audioSink, 1, ids, req);
    LOGD(TAG, "audioChannel  bqPlayerObject　混音器来播放声音");

    //初始化播放器
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    LOGD(TAG, "audioChannel  bqPlayerObject　初始化播放器");

    //获得播放数据队列操作接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);

    //设置回调(启动播放器后执行回调来获取数据并播放)
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);

    LOGD(TAG, "audioChannel  bqPlayerObject　启动播放器后执行回调来获取数据并播放");

    //获取播放状态接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerInterface);
    // 设置播放状态
    (*bqPlayerInterface)->SetPlayState(bqPlayerInterface, SL_PLAYSTATE_PLAYING);

    //还要手动调用一次 回调方法，才能开始播放
    bqPlayerCallback(bqPlayerBufferQueue, this);
    LOGD(TAG, "audioChannel  bqPlayerCallback　手动调一次回调方法，开始播放");
}

int AudioChannel::_getData() {
    int dataSize = 0;
    AVFrame *avFrame = nullptr;
    while (isPlaying) {
        int ret = frameQueue.deQueue(avFrame);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }

        //nb: 转换,每个通道输出的样本数，错误为负值
        int nb = swr_convert(swrContext, &buffer, bufferCount,
                             (const uint8_t **) avFrame->data, avFrame->nb_samples);

        // 采样位:out_sampleSize;声道数:out_channels
        dataSize = nb * out_channels * out_sampleSize;

        //播放这一段声音的时刻
        clock = avFrame->pts*av_q2d(timeBase);
        break;
    }
    releaseAvFrame(avFrame);
    return dataSize;
}

void AudioChannel::_releaseOpenSL() {
    LOGE(TAG,"停止播放");
    //设置停止状态
    if(bqPlayerInterface){
        (*bqPlayerInterface)->SetPlayState(bqPlayerInterface,SL_PLAYSTATE_STOPPED);
        bqPlayerInterface = nullptr;
    }
    //销毁播放器
    if (bqPlayerObject){
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = nullptr;
        bqPlayerBufferQueue = nullptr;
    }

    //销毁混音器
    if (outputMixObject){
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = nullptr;
    }

    //销毁引擎
    if (engineObject){
        (*engineObject)->Destroy(engineObject);
        engineObject = nullptr;
        engineInterface = nullptr;
    }

}
