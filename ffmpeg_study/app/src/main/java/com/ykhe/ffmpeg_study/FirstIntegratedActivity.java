package com.ykhe.ffmpeg_study;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

//Android Studio第一次集成ffmpeg 示例
public class FirstIntegratedActivity extends AppCompatActivity {

    TextView tv;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_first_integrated);
//
        // Example of a call to a native method
        tv = findViewById(R.id.sample_text);
        tv.setText(native_GetFFmpegVersion());
    }

    private static native String native_GetFFmpegVersion();
}