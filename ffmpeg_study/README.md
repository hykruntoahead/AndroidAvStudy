## Android 音视频开发-ffmpeg

### 0.FFmpeg 编译和集成

 FFmpeg 有六个常用的功能模块：

    - libavformat：多媒体文件或协议的封装和解封装库；
    - libavcodec：音视频编解码库；
    - libavfilter：音视频、字幕滤镜库；
    - libswscale：图像格式转换库；
    - libswresample：音频重采样库；
    - libavutil：工具库。


#### 0.1 编译

 编译环境:
    CentOS Linux release 7.6.1810 (Core)
    android-ndk-r20b-linux-x86_64
    ffmpeg-4.2.2

 编译前准备：

 ```
   #1. 下载 ffmpeg-4.2.2
   wget https://ffmpeg.org/releases/ffmpeg-4.2.2.tar.bz2
   #2. 解压 FFmpeg
   tar -jxvf ffmpeg-4.2.2.tar.bz2
   #3. 运行 configure 脚本配置项目
   ./configure --disable-x86asm

   ...
   #4.　下载ndk android-ndk-r20b-linux-x86_64
      https://dl.google.com/android/repository/android-ndk-r20b-linux-x86_64.zip?hl=zh_cn
 ```

　在 FFmpeg 4.2.2 解压目录下创建编译脚本 build_android_arm64-v8a_clang.sh：

```
	#!/bin/bash

    export NDK=/root/workspace/android-ndk-r20b #这里配置先你的 NDK 路径
    TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64


    function build_android
    {

    ./configure \
    --prefix=$PREFIX \
    --enable-neon  \
    --enable-hwaccels  \
    --enable-gpl   \
    --disable-postproc \
    --disable-debug \
    --enable-small \
    --enable-jni \
    --enable-mediacodec \
    --enable-decoder=h264_mediacodec \
    --enable-static \
    --enable-shared \
    --disable-doc \
    --enable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
    --disable-avdevice \
    --disable-doc \
    --disable-symver \
    --cross-prefix=$CROSS_PREFIX \
    --target-os=android \
    --arch=$ARCH \
    --cpu=$CPU \
    --cc=$CC \
    --cxx=$CXX \
    --enable-cross-compile \
    --sysroot=$SYSROOT \
    --extra-cflags="-Os -fpic $OPTIMIZE_CFLAGS" \
    --extra-ldflags="$ADDI_LDFLAGS"

    make clean
    make -j16
    make install

    echo "============================ build android arm64-v8a success =========================="

    }

    #arm64-v8a
    ARCH=arm64
    CPU=armv8-a
    API=21
    CC=$TOOLCHAIN/bin/aarch64-linux-android$API-clang
    CXX=$TOOLCHAIN/bin/aarch64-linux-android$API-clang++
    SYSROOT=$NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot
    CROSS_PREFIX=$TOOLCHAIN/bin/aarch64-linux-android-
    PREFIX=$(pwd)/android/$CPU
    OPTIMIZE_CFLAGS="-march=$CPU"

    build_android
```
  编译 FFmpeg Android 平台的 64 位动态库和静态库：

```
   # 修改 build_android_arm64-v8a_clang.sh 可执行权限
   chmod +x build_android_arm64-v8a_clang.sh
   # 运行编译脚本
   ./build_android_arm64-v8a_clang.sh
```
  编译成功后有日志　build android arm64-v8a success打印．
  cd 到　android/armv8-a/lib/下即可看到编译的so,a文件(对应六个模块的静态库和动态库).

  若要编译成 32 位的库，则需修改对应的编译脚本：
```
	#armv7-a
	ARCH=arm
	CPU=armv7-a
	API=21
	CC=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang
	CXX=$TOOLCHAIN/bin/armv7a-linux-androideabi$API-clang++<font color="#dd0000">
	SYSROOT=$NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot
	CROSS_PREFIX=$TOOLCHAIN/bin/arm-linux-androideabi-
	PREFIX=$(pwd)/android/$CPU
	OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=vfp -marm -march=$CPU "
```

### -1.Android 视频解码器
  // 将输入的视频数据解码成YUV像素数据
  > ffmpeg_study/app/src/main/cpp/learn_ffmpeg.cpp


### 1. Android FFmpeg视频解码播放

> 基于 FFmpeg 4.x 的音视频解码流程，重点讲解如何实现视频的播放。

[Android NDK入门:C++基础](https://www.jianshu.com/p/ea1be023d41c)



#### 1.1 FFmpeg 相关库简介
| 库 | 介绍 |
| --- | --- |
| avcodec | 音视频编解码核心库 |
| avformat | 音视频容器格式的封装和解析 |
| avutil | 核心工具库 |
| swscal | 图像格式转换的模块 |
| swresample | 音频重采样　｜
| avfilter | 音视频滤镜库，如视频加水印，音频变声 |
| avdevice | 输入输出设备库，提供设备数据的输入与输出 |
| postproc | 用于后期效果的处理 |

FFmpeg 就是依靠以上几个库，实现了强大的音视频**编码、解码、编辑、转换、采集**等能力.

**FFmpeg实际上也是一个引擎,能够集成包括librtmp,libmap3lame等第三方库,以FFmpeg统一接口使用.**

#### 1.2 FFmpeg 解码流程简介

有以下的流程：

    初始化解码器
    读取 Mp4 文件中的编码数据，并送入解码器解码
    获取解码好的帧数据
    将一帧画面渲染到屏幕上.

> FFmpeg 是利用 CPU 的计算能力来解码而已。

##### 1.2.1 FFmpeg 初始化

FFmpeg 初始化的流程相对　Android 原生硬解码来说还是比较琐碎的，
但是流程都是固定的，一旦封装起来就可以直接套用了．

- 初始化开始

- 从音视频文件中提取解码流信息:
	- 获取格式上下文(format_ctx:avformat_alloc_context)
	- 打开文件，并初始化(format_ctx:avformat_open_input)
	- 提取流信息到(format_ctx:avformat_find_stream_info)
	- 查找音/视频流索引(idx)
- 根据音视频流信息，初始化并打开解码器:
	- 通过format_ctx,idx获取音/视频　解码参数　code_par
	- 通过code_par 查找解码器　(codec:avcodec_find_decoder)
	- 获取解码器上下文 (code_ctx: avcodec_parameters_to_context)
	-　打开解码器　(codec:avcodec_open2)

- 初始化结束

其中，有几个 结构体 比较重要，分别是 AVFormatContext(format_ctx)、AVCodecContext(codec_ctx)、AVCodec(codec)．

- AVFormatContext: 属于avformat库，存放着码流数据的上下文，主要用于音视频的*封装*和*解封*
- AVCodecContext: 属于avcodec库，存放编解码器参数上下文，主要用于对音视频数据进行*编码*和*解码*
- ACCodec: 属于avcodec库，音视频编解码器，真正的**编解码执行者**.

##### 1.2.2 FFmpeg　解码循环

解码过程：
1. 解码开始
2. 从音视频流中提取帧数据(avpacket:av_read_frame)->3;
3. 将帧数据avpacket送入解码(avcodec_send_packet)->4;
4. 提取解码完成的数据(frame:avcodec_receive_frame)->5;
5. 释放数据流packet(av_packet_unref) -> 6;
6. frame == null ? Y->8 : N->7;
7. 渲染 ->2;
8. 解码结束.

从上面流程可以看到，FFmpeg 首先将数据提取为一个 AVPacket（avpacket），然后通过解码，将数据解码为一帧可以渲染的数据，
称为 AVFrame（frame）．

> AVPacket 和 AVFrame 也是两个结构体，里面封装了具体的数据。

### 2 FFmpeg 播放器开发

#### 2.1  流程概述
```mermaid
graph TB;
　　封装格式数据-mp4,flv等-->|解封装|音频压缩数据-aac,mp3等
　　封装格式数据-mp4,flv等-->|解封装|视频压缩数据-aac,mp3等
　　音频压缩数据-aac,mp3等-->|音频解码|音频采样数据-pcm
　　视频压缩数据-aac,mp3等-->|视频解码|视频像素数据-yuv
　　音频采样数据-pcm.->音视频同步
　　视频像素数据-yuv.->音视频同步
　　音视频同步-->音频驱动/设备
　　音视频同步-->视频驱动/设备
```

#### 2.2 ffmpeg工具的使用

ffmpeg是一个非常快的视频/音频转换器，其也可以现场抓取音频/视频源，并在任意采样率、尺寸之间调整视频，以及提供多种高品质的滤镜系统。

- ffmpeg 对视频进行各种处理(转码、缩放等)；
- ffplay播放器；
- ffprobe 查看多媒体文件的信息。

缩放: ffmpeg -i input.mp4 -s 100x100 output.mp4
播放：ffplay -i input.mp4
查看：ffprobe -i input.mp4

[ffmpeg 翻译文档](https://xdsnet.gitbooks.io/other-doc-cn-ffmpeg/content/index.html)

#### 2.3 主要结构体及函数

- AVPacket：解码前的数据结构体；
- AvFrame：解码后的数据结构体；
- AVFormatContext：媒体文件的构成和基本信息上下文；
- AVCodecContext: 解码信息上下文；



解码流程(主要方法):

```mermaid
graph TB
	start[Start] -->|开启网络支持|network_init[avformat_network_init]
	network_init -->|打开输入媒体文件|open_input[avformat_open_input]
	open_input -->|获取媒体流信息|find_stream[avformat_find_stream_info]
	find_stream -->|查找匹配解码器|find_decoder[avcodec_find_decoder]
	find_decoder -->|打开解码器|open_decoder[avcodec_open2]
	open_decoder -->|从输入文件读取一帧压缩数据|read_frame[av_read_frame]
	read_frame -->get_package{Get Packet?}
	get_package -->|false|close
	get_package -->|true:获取压缩数据|av_packet[AVPacket]
	
	
	av_packet -->|发送给解码器|send_packet[avcodec_send_packet]
	send_packet -->|从解码器获得解码数据|receive_frame[avcodec_receive_frame]
	receive_frame -->|获取音视频原始数据|av_frame[AVFrame]
	av_frame -->|Show on Display|read_frame
   
```
