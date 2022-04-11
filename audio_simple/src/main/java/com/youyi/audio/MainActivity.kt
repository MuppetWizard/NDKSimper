package com.youyi.audio

import android.annotation.SuppressLint
import android.content.res.AssetManager
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.provider.MediaStore
import android.util.Log
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import com.youyi.audio.databinding.ActivityMainBinding

/**
 * des：
 * @author: Muppet
 * @date: 2022/4/8
 */

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    private lateinit var mAudioTracker: AudioTracker

    @SuppressLint("NewApi")
    private val PATH = "/storage/sdcard0/_test.pcm"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        initAudioTrack()
    }

    private fun initAudioTrack() {
        mAudioTracker = AudioTracker(this)
//        mAudioTracker.createAudioTrack()
        Log.d("MyPath",PATH)
    }


}