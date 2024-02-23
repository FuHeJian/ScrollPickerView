package com.fhj.mvi_demo;

import android.os.Bundle;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

/**
 * @author: fuhejian
 * @date: 2024/2/23
 */
public class SecondActivity extends AppCompatActivity {

    private String TAG = "日志";
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "2-onCreate: ");
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.d(TAG, "2-onStart: ");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "2-onResume: ");
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.d(TAG, "2-onStop: ");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "2-onDestroy: ");
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d(TAG, "2-onPause: ");
    }

}
