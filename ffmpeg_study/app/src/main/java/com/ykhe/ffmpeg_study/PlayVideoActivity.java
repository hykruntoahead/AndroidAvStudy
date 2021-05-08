package com.ykhe.ffmpeg_study;

import android.os.Bundle;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

/**
 * author: ykhe
 * date: 21-5-8
 * email: ykhe@grandstream.cn
 * description:
 */
public class PlayVideoActivity extends AppCompatActivity {

    static {
        System.loadLibrary("native-lib-paly-video");
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

    }
}
