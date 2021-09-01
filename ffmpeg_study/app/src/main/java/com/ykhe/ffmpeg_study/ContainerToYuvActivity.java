package com.ykhe.ffmpeg_study;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.widget.Button;
import android.widget.EditText;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

/**
 * author: ykhe
 * date: 21-7-15
 * email: ykhe@grandstream.cn
 * description:
 */
public class ContainerToYuvActivity extends AppCompatActivity {

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_to_yuv);

        Button startButton = (Button) this.findViewById(R.id.bt_start);
        final EditText urlEdittext_input = (EditText) this.findViewById(R.id.et_input);
        final EditText urlEdittext_output = (EditText) this.findViewById(R.id.et_output);

        startButton.setOnClickListener(arg0 -> {

            String folderurl = getFilesDir().getAbsolutePath();

            String urltext_input = urlEdittext_input.getText().toString();
            String inputurl = folderurl + "/" + urltext_input;

            String urltext_output = urlEdittext_output.getText().toString();
            String outputurl = folderurl + "/" + urltext_output;

            Log.i("ContainerToYuv","inputurl:"+ inputurl);
            Log.i("ContainerToYuv","outputurl:"+ outputurl);

            decode(inputurl, outputurl);
        });
    }

    //JNI
    public native int decode(String inputurl, String outputurl);
}
