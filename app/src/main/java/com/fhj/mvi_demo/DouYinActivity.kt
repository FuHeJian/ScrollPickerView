package com.fhj.mvi_demo

import android.os.Bundle
import android.view.Surface
import androidx.appcompat.app.AppCompatActivity
import com.fhj.mvi_demo.databinding.DouyinPageBinding

/**
@author: fuhejian
@date: 2024/1/22
 */
class DouYinActivity: AppCompatActivity() {

    lateinit var binding: DouyinPageBinding
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = DouyinPageBinding.inflate(layoutInflater)
        binding.surfaceView.holder.surface
    }

    override fun onStart() {
        super.onStart()
    }

    init {
        System.loadLibrary("test")
    }

    external fun op(p: String, surface: Surface?): Int

}