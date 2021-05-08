package com.ykhe.ffmpeg_study;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

//Android Studio第一次集成ffmpeg 示例
public class FirstIntegratedActivity extends AppCompatActivity implements View.OnClickListener {

    TextView tv;
    Button protocol;
    Button format;
    Button codec;
    Button filter;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_first_integrated);

        protocol = (Button) findViewById(R.id.btn_protocol);
        format = (Button) findViewById(R.id.btn_format);
        codec = (Button) findViewById(R.id.btn_codec);
        filter = (Button) findViewById(R.id.btn_filter);
        protocol.setOnClickListener(this);
        format.setOnClickListener(this);
        codec.setOnClickListener(this);
        filter.setOnClickListener(this);

        // Example of a call to a native method
        tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());
    }


    public native String stringFromJNI();

    public native String urlprotocolinfo();

    public native String avformatinfo();

    public native String avcodecinfo();

    public native String avfilterinfo();

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_protocol:
                tv.setText(urlprotocolinfo());
                break;
            case R.id.btn_format:
                tv.setText(avformatinfo());
                break;
            case R.id.btn_codec:
                tv.setText(avcodecinfo());
                break;
            case R.id.btn_filter:
                tv.setText(avfilterinfo());
                break;
            default:
                break;
        }
    }
}