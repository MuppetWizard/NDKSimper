package com.youyi.video

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
import com.youyi.video.databinding.ActivityMainBinding
import java.io.File

/**
 * des：
 * @author: Muppet
 * @date: 2022/4/8
 */

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding


    @SuppressLint("NewApi")
    private val PATH = "${Environment.getExternalStorageDirectory().absolutePath}/_test.pcm"

    companion object{
        init {
            System.loadLibrary("audio-simple")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        initView()
        initAudioTrack()
        requestPermission()

    }

    override fun onDestroy() {
        super.onDestroy()
        nativeStopPCM()
    }

    private fun initAudioTrack() {

    }

    private fun initView() {

        binding.btnOpenslStart.setOnClickListener {
            nativePlayPCM(PATH)
        }
        binding.btnOpenslStop.setOnClickListener {
            nativeStopPCM()
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

    private external fun nativePlayPCM(path: String)

    private external fun nativeStopPCM()

}