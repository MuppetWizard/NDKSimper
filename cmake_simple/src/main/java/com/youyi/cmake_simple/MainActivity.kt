package com.youyi.cmake_simple

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        testCmake()
    }

    external fun testCmake();

    companion object{
        init {
            System.loadLibrary("Test")
            System.loadLibrary("native-ffmpeg.lib")
        }
    }

}