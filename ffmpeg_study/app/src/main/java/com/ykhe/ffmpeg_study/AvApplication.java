package com.ykhe.ffmpeg_study;

import android.app.Application;

/**
 * author: ykhe
 * date: 21-9-1
 * email: ykhe@grandstream.cn
 * description:
 */
public class AvApplication extends Application {
    static {
        System.loadLibrary("learn-ffmpeg");
    }

}
