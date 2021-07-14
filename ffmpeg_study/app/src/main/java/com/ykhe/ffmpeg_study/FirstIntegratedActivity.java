package com.ykhe.ffmpeg_study;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

//Android Studio第一次集成ffmpeg 示例
public class FirstIntegratedActivity extends AppCompatActivity {

    TextView tv;
    Button protocol;
    Button format;
    Button codec;
    Button filter;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_first_integrated);
//
//        protocol = (Button) findViewById(R.id.btn_protocol);
//        format = (Button) findViewById(R.id.btn_format);
//        codec = (Button) findViewById(R.id.btn_codec);
//        filter = (Button) findViewById(R.id.btn_filter);
//        protocol.setOnClickListener(this);
//        format.setOnClickListener(this);
//        codec.setOnClickListener(this);
//        filter.setOnClickListener(this);

        // Example of a call to a native method
        tv = findViewById(R.id.sample_text);
        tv.setText(native_GetFFmpegVersion());
    }

    static {
        System.loadLibrary("learn-ffmpeg");
    }


    private static native String native_GetFFmpegVersion();
}