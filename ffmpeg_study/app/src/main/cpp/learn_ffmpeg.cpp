#include <cstdio>
#include <cstring>
#include "util/LogUtil.h"
#include "jni.h"

//由于 FFmpeg 库是 C 语言实现的，告诉编译器按照 C 的规则进行编译


#define TAG = "ContainerToYuv"


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

/*
 * Class:     com_byteflow_learnffmpeg_media_FFMediaPlayer
 * Method:    native_GetFFmpegVersion
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_com_ykhe_ffmpeg_1study_FirstIntegratedActivity_native_1GetFFmpegVersion
        (JNIEnv *env, jclass cls) {
    char strBuffer[1024 * 4] = {0};
    strcat(strBuffer, "libavcodec : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVCODEC_VERSION));
    strcat(strBuffer, "\nlibavformat : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVFORMAT_VERSION));
    strcat(strBuffer, "\nlibavutil : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVUTIL_VERSION));
    strcat(strBuffer, "\nlibavfilter : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVFILTER_VERSION));
    strcat(strBuffer, "\nlibswresample : ");
    strcat(strBuffer, AV_STRINGIFY(LIBSWRESAMPLE_VERSION));
    strcat(strBuffer, "\nlibswscale : ");
    strcat(strBuffer, AV_STRINGIFY(LIBSWSCALE_VERSION));
    strcat(strBuffer, "\navcodec_configure : \n");
    strcat(strBuffer, avcodec_configuration());
    strcat(strBuffer, "\navcodec_license : ");
    strcat(strBuffer, avcodec_license());
    LOGI("INFO", "GetFFmpegVersion\n%s", strBuffer);
    return env->NewStringUTF(strBuffer);
}


//Output FFmpeg's av_log()
void custom_log(void *ptr, int level, const char *fmt, va_list vl) {
    FILE *fp = fopen("/storage/emulated/0/av_log.txt", "a+");
    if (fp) {
        vfprintf(fp, fmt, vl);
        fflush(fp);
        fclose(fp);
    }
}


JNIEXPORT jint JNICALL
Java_com_ykhe_ffmpeg_1study_ContainerToYuvActivity_decode(JNIEnv *env, jobject thiz,
                                                          jstring input_jstr, jstring output_jstr) {

    AVFormatContext *pFormatCtx;
    int i, videoindex;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameYUV;
    uint8_t *out_buffer;
    AVPacket *packet;
    int y_size;
    int ret, got_picture;
    struct SwsContext *img_convert_ctx;
    FILE *fp_yuv;
    int frame_cnt;
    clock_t time_start, time_finish;
    double time_duration = 0.0;

    char input_str[500] = {0};
    char output_str[500] = {0};
    char info[1000] = {0};
    const char* utfInputStr = (*env).GetStringUTFChars(/*env, */input_jstr, NULL);
    const char* utfOutputStr = (*env).GetStringUTFChars(/*env, */output_jstr, NULL);
    LOGI("ContainerToYuv","input:%s;output:%s",utfInputStr,utfOutputStr);
    sprintf(input_str, "%s", utfInputStr);
    sprintf(output_str, "%s", utfOutputStr);

    //FFmpeg av_log() callback
    av_log_set_callback(custom_log);

    //初始化所有组件，只有调用了该函数，才能使用复用器和编解码器

    //，该接口内部的调用为：
    //
    //(1).avcodec_register_all()，该接口内部执行步骤：
    //
    // - 注册硬件加速器：REGISTER_HWACCEL()
    //
    // - 注册音视频编码器：REGISTER_ENCODER()
    //
    // - 注册音视频解码器：REGISTER_DECODER()
    //
    // - 打包注册：REGISTER_ENCDEC()
    //
    // - 注册解析器：REGISTER_PARSER()
    //
    //(2).执行复用器和解复用器的注册：
    //
    // - 注册复用器：REGISTER_MUXER()
    //
    // - 注册解复用器：REGISTER_DEMUXER()
    //
    // - 打包注册：REGISTER_MUXDEMUX()

    av_register_all();
    LOGI("ContainerToYuv","av_register_all");
    //初始化网络组件
    avformat_network_init();
    LOGI("ContainerToYuv","avformat_network_init");
    //获取解封装上下文　AVFormatContext
    pFormatCtx = avformat_alloc_context();
    LOGI("ContainerToYuv","avformat_alloc_context");

    /**
     * 打开多媒体数据并且获得一些相关的信息:
     *
     * ps：函数调用成功之后处理过的AVFormatContext结构体。
     * file：打开的视音频流的URL。
     * fmt：强制指定AVFormatContext中AVInputFormat的。这个参数一般情况下可以设置为NULL，这样FFmpeg可以自动检测AVInputFormat。
     * dictionay：附加的一些选项，一般情况下可以设置为NULL。
     */
    if (avformat_open_input(&pFormatCtx, input_str, NULL, NULL) != 0) {
        LOGE("ContainerToYuv", "Couldn't open input stream.\n");
        return -1;
    }

    /**
     * 读取一部分视音频数据并且获得一些相关的信息
     *
     * ic：输入的AVFormatContext
     * options：额外的选项
     */
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("ContainerToYuv", "Couldn't find stream information.\n");
        return -1;
    }

    videoindex = -1;
    //获取视频流
    for (i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            break;
        }
    if (videoindex == -1) {
        LOGE("ContainerToYuv", "Couldn't find a video stream.\n");
        return -1;
    }
    LOGI("ContainerToYuv","start codec");
    // 获取解码上下文
    pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    // 获取解码器
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        LOGE("ContainerToYuv", "Couldn't find Codec.\n");
        return -1;
    }
    // 打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGE("ContainerToYuv", "Couldn't open codec.\n");
        return -1;
    }

    // 分配一个 AVFrame 并将其字段设置为默认值。 必须使用 av_frame_free() 释放结构体。
    //  *
    //  * @return 一个 AVFrame 填充默认值或失败时为 NULL。
    //  *
    //  * @note 这只会分配 AVFrame 本身，而不是数据缓冲区。
    //  * 那些必须通过其他方式分配，例如 使用 av_frame_get_buffer() 或
    //  * 手动分配。

    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    /**
     * 返回存储所需数据量的字节大小
     * 带有给定参数的图像。
     * * @param pix_fmt 图像的像素格式  YUV420P在内存中的排布如下: YYYYYYYY UUUU VVVV
     * @param width 图像的宽度（以像素为单位）
     * @param height 以像素为单位的图像高度
     * @param align 假设的linesize 对齐方式
     * @return 缓冲区大小（以字节为单位），失败时返回负错误代码
     * av_image_get_buffer_size(enum AVPixelFormat pix_fmt, int width, int height, int align)
     *
     *
     * 分配一个适合所有内存访问的对齐方式的内存块（包括矢量，如果在 CPU 上可用)
     * @param size 要分配的内存块的大小（以字节为单位）
     * @return 指向已分配块的指针，如果块不能，则为`NULL`被分配
     */

    out_buffer = (unsigned char *) av_malloc(
            av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));

    LOGI("ContainerToYuv","av_image_get_buffer_size");

    /**
     *
    * 根据指定图像设置数据指针和线宽参数和提供的数组。
    * 给定图像的字段使用 src 填充指向图像数据缓冲区的地址。
     * 取决于指定的像素格式，一个或多个图像数据指针和将设置行大小。
     * 如果指定了平面格式，则有几个指针将被设置为指向不同的画面平面和不同平面的线尺寸将存储在lines_sizes 数组。
     * 使用 src == NULL 调用以获得所需的 src 缓冲区的大小。
 *
 * 分配缓冲区并填入dst_data和dst_linesize
 * 一次调用，使用 av_image_alloc()。
 *
 * @param dst_data 需要填写的数据指针
 * @param dst_linesize 要填充的 dst_data 中图像的线尺寸
 * @param src 缓冲区将包含或包含实际图像数据，可以为 NULL
 * @param pix_fmt 图像的像素格式
 * @param width 图像的宽度（以像素为单位）
 * @param height 图像的高度（以像素为单位）
 * @param align  中用于 linesize 对齐的值
 * @return 所需的字节大小， 在失败的情况下负错误代码
 *
 */

    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    LOGI("ContainerToYuv","av_image_fill_arrays");

    // AVPacket是FFmpeg中很重要的一个数据结构
    // 它保存了 解复用之后，解码之前的数据（仍然是压缩后的数据）和关于这些数据的一些附加信息，
    // 如显示时间戳（pts）、解码时间戳（dts）、数据时长，所在媒体流的索引等。
    packet = (AVPacket *) av_malloc(sizeof(AVPacket));

/**
 * 分配并返回一个 SwsContext。你需要它来执行使用 sws_scale() 进行缩放/转换操作。
 *
 * @param srcW 源图像的宽度
 * @param srcH 源图像的高度
 * @param srcFormat 源图片格式
 * @param dstW 目标图像的宽度
 * @param dstH 目标图像的高度
 * @param dstFormat 目标图片格式
 * @param 标志指定用于重新缩放的算法和选项
 * @param param 额外参数来调整使用的缩放器
 * 对于 SWS_BICUBIC param[0] 和 [1] 调整基础的形状函数，param[0] 调整 f(1) 和 param[1] f´(1)
 * 对于 SWS_GAUSS param[0] 调整指数，从而截止频率
 * 对于 SWS_LANCZOS param[0] 调整窗口函数的宽度
 * @return 指向已分配上下文的指针，如果出错则返回 NULL
 * @note 此功能将在更明智的选择后被删除
 * 写的
 */
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                     pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                     SWS_BICUBIC, NULL, NULL, NULL);
    LOGI("ContainerToYuv","sws_getContext");

    sprintf(info, "[Input     ]%s\n", input_str);
    sprintf(info, "%s[Output    ]%s\n", info, output_str);
    sprintf(info, "%s[Format    ]%s\n", info, pFormatCtx->iformat->name);
    sprintf(info, "%s[Codec     ]%s\n", info, pCodecCtx->codec->name);
    sprintf(info, "%s[Resolution]%dx%d\n", info, pCodecCtx->width, pCodecCtx->height);


    fp_yuv = fopen(output_str, "wb+");
    if (fp_yuv == NULL) {
        LOGE("ContainerToYuv","Cannot open output file.\n");
        return -1;
    }

    frame_cnt = 0;
    time_start = clock();

    LOGI("ContainerToYuv","start av_read_frame");
    /**
     * av_read_frame()的作用是读取码流中的音频若干帧或者视频一帧。
     * 例如，解码视频的时候，每解码一个视频帧，需要先调用 av_read_frame()获得一帧视频的压缩数据，
     * 然后才能对该数据进行解码.
     *
     *   s：输入的AVFormatContext
     *   pkt：输出的AVPacket
     *   如果返回0则说明读取正常
     */
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        LOGI("ContainerToYuv","av_read_frame > =0");
        if (packet->stream_index == videoindex) {
            /**
             * avcodec_decode_video2()的作用是解码一帧视频数据。
             * 输入一个压缩编码的结构体AVPacket，输出一个解码后的结构体AVFrame
             *
             * got_picture_ptr 如果没有可以解压缩的帧为零，否则为非零
             */
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if (ret < 0) {
                LOGE("ContainerToYuv", "Decode Error.\n");
                return -1;
            }

            LOGI("ContainerToYuv","avcodec_decode_video2:videoIndex=%d,ret=%d,got_picture=%d",videoindex,ret,got_picture);
            if (got_picture) {
                /**
                 *   在 srcSlice 中缩放图像切片并将结果缩放 在 dst 中对图像进行切片。
                 *   一个切片是一个连续的序列 图像中的行。
                 * 切片必须按顺序提供，无论是在 自上而下或自下而上的顺序。
                 * 如果切片在 non-sequential order 函数的行为是未定义的。
                *
                * @param c 之前创建的缩放上下文 sws_getContext()
                * @param srcSlice 包含指向平面的指针的数组源切片
                * @param srcStride 包含每个平面的步幅的数组源图像
                * @param srcSliceY 切片在源图像中的位置进程，即数量（从零）在切片第一行的图像中
                * @param srcSliceH 源切片的高度，即数字切片中的行数
                * @param dst 包含指向平面的指针的数组目标图像
                * @param dstStride 包含每个平面的步幅的数组目标图像
                * @return 输出切片的高度
                 *
                 * */
                sws_scale(img_convert_ctx, (const uint8_t *const *) pFrame->data, pFrame->linesize,
                          0, pCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);
                y_size = pCodecCtx->width * pCodecCtx->height;

                LOGI("ContainerToYuv","sws_scale:width:%d,height:%d,size:%d",pCodecCtx->width,pCodecCtx->height,y_size);
                //将yuv数据写入文件
                /**
                 * C 库函数 size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
                 * 把 ptr 所指向的数组中的数据写入到给定流 stream 中.
                 *  ptr -- 这是指向要被写入的元素数组的指针。
                 *  size -- 这是要被写入的每个元素的大小，以字节为单位。
                 *  nmemb -- 这是元素的个数，每个元素的大小为 size 字节。
                 *  stream -- 这是指向 FILE 对象的指针，该 FILE 对象指定了一个输出流。
                 *
                 *
                 *  YUV420P(planar格式)在ffmpeg中存储是在struct AVFrame的data[]数组中
                 *  data[0]-------Y分量
                 *  data[1]------U分量
                 *  data[2]-------V分量
                 *  YUV420P的内存结构: 4个Y分量对应1个UV分量
                 */
                fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y
                fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U
                fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V

                LOGI("ContainerToYuv","fwrite YUV");
                //Output info
                char pictype_str[10] = {0};
                switch (pFrame->pict_type) {
                    case AV_PICTURE_TYPE_I:
                        sprintf(pictype_str, "I");
                        break;
                    case AV_PICTURE_TYPE_P:
                        sprintf(pictype_str, "P");
                        break;
                    case AV_PICTURE_TYPE_B:
                        sprintf(pictype_str, "B");
                        break;
                    default:
                        sprintf(pictype_str, "Other");
                        break;
                }
                frame_cnt++;
                LOGI("ContainerToYuv","Frame Index: %5d. Type:%s ;frame_cnt:%d", frame_cnt, pictype_str,frame_cnt);
            }
        }
        /**
         * 释放AVPacket
         */
        av_free_packet(packet);
        LOGI("ContainerToYuv","av_free_packet");
    }

    //flush decoder
    //FIX: Flush Frames remained in Codec

    LOGI("ContainerToYuv","Flush Frames remained in Codec");
    while (1) {
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
        if (ret < 0)
            break;
        if (!got_picture)
            break;

        sws_scale(img_convert_ctx, (const uint8_t *const *) pFrame->data, pFrame->linesize, 0,
                  pCodecCtx->height,
                  pFrameYUV->data, pFrameYUV->linesize);
        int y_size = pCodecCtx->width * pCodecCtx->height;
        fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y
        fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U
        fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V
        //Output info
        char pictype_str[10] = {0};
        switch (pFrame->pict_type) {
            case AV_PICTURE_TYPE_I:
                sprintf(pictype_str, "I");
                break;
            case AV_PICTURE_TYPE_P:
                sprintf(pictype_str, "P");
                break;
            case AV_PICTURE_TYPE_B:
                sprintf(pictype_str, "B");
                break;
            default:
                sprintf(pictype_str, "Other");
                break;
        }
        frame_cnt++;
        LOGI("ContainerToYuv","Frame Index: %5d. Type:%s,frame_cnt:%d", frame_cnt, pictype_str,frame_cnt);
    }
    time_finish = clock();
    time_duration = (double) (time_finish - time_start);

    sprintf(info, "%s[Time      ]%fms\n", info, time_duration);
    sprintf(info, "%s[Count     ]%d\n", info, frame_cnt);

    LOGI("ContainerToYuv","free close");
    sws_freeContext(img_convert_ctx);

    fclose(fp_yuv);

    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}

#ifdef __cplusplus
}
#endif
