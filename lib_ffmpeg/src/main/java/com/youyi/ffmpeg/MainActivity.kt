package com.youyi.ffmpeg

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.youyi.ffmpeg.databinding.ActivityMainBinding

/**
 * des：
 * @author: Muppet
 * @date: 2022/4/8
 */

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.tvTest.text = "FFmpeg Version: ${getFFmpegVersion()}"
    }

    companion object{
        init {
            System.loadLibrary("ffmpeg_lib")
        }
        external fun getFFmpegVersion(): String
    }

}