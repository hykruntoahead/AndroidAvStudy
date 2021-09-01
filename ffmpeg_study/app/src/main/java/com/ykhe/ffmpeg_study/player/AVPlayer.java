package com.ykhe.ffmpeg_study.player;

import android.util.Log;

/**
 * author: ykhe
 * date: 21-9-1
 * email: ykhe@grandstream.cn
 * description:
 */
public class AVPlayer {
    private static final String TAG = "AVPlayer";

    //native层 AVPlayer句柄
    private long nativeHandle;

    public AVPlayer(){
       nativeHandle = nativeInit();
       Log.d(TAG, "AVPlayer: initAvPlayer and handle is:"+nativeHandle);
    }

    public void setDataSource(String path){
        setDataSource(nativeHandle,path);
    }

    public void prepare(){
        prepare(nativeHandle);
    }

    private void  start(){

    }

    private void stop(){

    }

    private void pause(){

    }

    private native long nativeInit();
    private native void setDataSource(long nativeHandle,String path);
    private native void prepare(long nativeHandle);
}
