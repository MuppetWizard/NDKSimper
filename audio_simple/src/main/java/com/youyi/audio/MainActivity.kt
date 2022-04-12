package com.youyi.audio

import android.Manifest
import android.annotation.SuppressLint
import android.app.Activity
import android.content.pm.PackageManager
import android.content.res.AssetManager
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.provider.MediaStore
import android.util.Log
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.youyi.audio.databinding.ActivityMainBinding
import java.io.File

/**
 * des：
 * @author: Muppet
 * @date: 2022/4/8
 */

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    private lateinit var mAudioTracker: AudioTracker

    @SuppressLint("NewApi")
    private val PATH = "${Environment.getExternalStorageDirectory().absolutePath}/_test.pcm"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        initView()
        initAudioTrack()
        requestPermission()

    }

    private fun initAudioTrack() {
        mAudioTracker = AudioTracker(this)
        mAudioTracker.createAudioTrack(PATH)
    }

    private fun initView() {
        binding.btnAtPlay.setOnClickListener {
            mAudioTracker.start()
        }

        binding.btnAtStop.setOnClickListener {
            mAudioTracker.stop()
        }
    }

    private fun requestPermission() {
        if (ActivityCompat.checkSelfPermission(
                this,
                Manifest.permission.READ_EXTERNAL_STORAGE
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE,Manifest.permission.WRITE_EXTERNAL_STORAGE),100)
        }
    }

}