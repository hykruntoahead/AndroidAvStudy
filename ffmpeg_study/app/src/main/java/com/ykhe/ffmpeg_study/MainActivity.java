package com.ykhe.ffmpeg_study;

import android.content.Intent;
import android.os.Bundle;
import android.util.ArrayMap;
import android.view.View;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.ykhe.ffmpeg_study.player.PlayerActivity;

/**
 * author: ykhe
 * date: 21-5-8
 * email: ykhe@grandstream.cn
 * description:
 */
public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    ArrayMap<Integer,Class> arrayMap = new ArrayMap<>();

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        adaptBtnLaunchActivity();
        addBtnClickListeners();
    }


    private void adaptBtnLaunchActivity() {
        arrayMap.put(R.id.btn_frist_integrated,FirstIntegratedActivity.class);
        arrayMap.put(R.id.btn_toYuv, ContainerToYuvActivity.class);
        arrayMap.put(R.id.btn_player, PlayerActivity.class);
    }

    private void addBtnClickListeners() {
        for (int btnId : arrayMap.keySet()) {
            addBtnClickListener(btnId);
        }
    }


    private void addBtnClickListener(int btnId) {
        findViewById(btnId).setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        startActivity(new Intent(this,arrayMap.get(v.getId())));
    }
}
