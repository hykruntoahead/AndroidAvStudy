package com.ykhe.ffmpeg_study.player;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.ykhe.ffmpeg_study.R;

import java.io.File;

/**
 * author: ykhe
 * date: 21-9-1
 * email: ykhe@grandstream.cn
 * description:
 */
public class PlayerActivity extends AppCompatActivity {
    private static final String TAG = "PlayerActivity";

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_player);
        AVPlayer avPlayer = new AVPlayer();
        String videoPath = getFilesDir() + File.separator +"1.mp4";
        Log.d(TAG, "onCreate: "+videoPath);
        avPlayer.setDataSource(videoPath);

        avPlayer.prepare();
    }
}
