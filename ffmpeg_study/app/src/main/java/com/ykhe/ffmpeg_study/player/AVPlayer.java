package com.ykhe.ffmpeg_study.player;

import android.util.Log;
import android.view.Surface;

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

    private OnErrorListener errorListener;
    private OnPreparedListener preparedListener;
    private OnProgressListener progressListener;


    public AVPlayer(){
        nativeHandle = nativeInit();
        Log.d(TAG, "AVPlayer: initAvPlayer and handle is:"+nativeHandle);
    }

    public void setOnErrorListener(OnErrorListener errorListener) {
        this.errorListener = errorListener;
    }

    public void setOnPreparedListener(OnPreparedListener preparedListener) {
        this.preparedListener = preparedListener;
    }

    public void setOnProgressListener(OnProgressListener progressListener) {
        this.progressListener = progressListener;
    }


    public void setDataSource(String path){
        setDataSource(nativeHandle,path);
    }

    public void prepare(){
        prepare(nativeHandle);
    }

    public void start(){
        start(nativeHandle);
    }

    public void stop(){

    }

    public void pause(){

    }


    private void onError(int errorCode){
        if (errorListener!=null){
            errorListener.onError(errorCode);
        }
    }

    private void onPrepared(){
        if (preparedListener!=null){
            preparedListener.onPrepared();
        }
    }

    private void onProgress(int progress){
        if (progressListener!=null){
            progressListener.onProgress(progress);
        }
    }

    public void setSurface(Surface surface) {
        setSurface(nativeHandle,surface);
    }

    public interface OnErrorListener{
        void onError(int error);
    }

    public interface OnPreparedListener{
        void onPrepared();
    }

    public interface OnProgressListener{
        void onProgress(int progress);
    }


    private native long nativeInit();
    private native void setDataSource(long nativeHandle,String path);
    private native void prepare(long nativeHandle);
    private native void start(long nativeHandle);
    private native void setSurface(long nativeHandle, Surface surface);
}
