package com.ykhe.ffmpeg_study.player;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;
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
public class PlayerActivity extends AppCompatActivity implements SurfaceHolder.Callback {
    private static final String TAG = "PlayerActivity";
    private AVPlayer avPlayer;
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_player);
        avPlayer = new AVPlayer();
        String videoPath = getFilesDir() + File.separator +"1.mp4";
//        String videoPath = "/sdcard/1.mp4";
        Log.d(TAG, "onCreate: "+videoPath);

        SurfaceView surfaceView = findViewById(R.id.surfaceView);
        surfaceView.getHolder().addCallback(this);

        avPlayer.setDataSource(videoPath);
        avPlayer.setOnPreparedListener(avPlayer::start);
        avPlayer.prepare();

    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
            Surface surface = holder.getSurface();
            avPlayer.setSurface(surface);
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {

    }
}
