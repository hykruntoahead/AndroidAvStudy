package com.ykhe.ffmpeg_study.player;

import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;

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
    private SurfaceView surfaceView;
    private String path;
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_player);
        avPlayer = new AVPlayer();
//        path = "https://upos-sz-mirrorcos.bilivideo.com/upgcxcode/60/42/408944260/408944260-1-192.mp4";
        path = getFilesDir() + File.separator +"1.mp4";
//        String videoPath = "/sdcard/1.mp4";
        Log.d(TAG, "onCreate: "+path);

        surfaceView = findViewById(R.id.surfaceView);
        surfaceView.getHolder().addCallback(this);

        avPlayer.setDataSource(path);
        avPlayer.setOnPreparedListener(avPlayer::start);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        surfaceView.getHolder().removeCallback(this);
        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager
                    .LayoutParams.FLAG_FULLSCREEN);
        } else {
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }
        setContentView(R.layout.activity_player);
        surfaceView = findViewById(R.id.surfaceView);
        surfaceView.getHolder().addCallback(this);
        avPlayer.setDataSource(path);
    }

    @Override
    protected void onResume() {
        super.onResume();
        avPlayer.prepare();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (avPlayer!=null){
            avPlayer.stop();
        }
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
