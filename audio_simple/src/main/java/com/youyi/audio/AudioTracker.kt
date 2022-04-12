package com.youyi.audio

import android.content.Context
import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioManager
import android.media.AudioTrack
import android.os.Build
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.widget.Toast
import java.io.BufferedInputStream
import java.io.DataInputStream
import java.io.FileInputStream
import java.io.InputStream
import java.lang.Exception
import java.lang.IllegalArgumentException
import java.util.concurrent.Executors


/**
 * des：
 * @author: Muppet
 * @date: 2022/4/11
 */
class AudioTracker(private val mContext: Context) {
    companion object{
        private const val TAG = "AudioTracker"
        //采样率 44100 hz
        private const val SAMPLE_RATE = 44100
        //单声道
        private const val CHANNEL = AudioFormat.CHANNEL_OUT_MONO
        //位深 16bit
        private const val AUDIO_FORMAT = AudioFormat.ENCODING_PCM_16BIT
    }
    //缓冲区大小
    private var mBufferSizeInByte = 0
    //播放对象
    private var mAudioTrack: AudioTrack? = null
    //文件名
    private var mFilePath: String = ""
    //状态
    private var mStatus:Status = Status.STATUS_NOT_READY
    //单任务线程池
    private val mExecutorService = Executors.newSingleThreadScheduledExecutor()

    private val mHandler: Handler = Handler(Looper.getMainLooper())



    @Throws(
        IllegalArgumentException::class,
    )
    fun createAudioTrack(filePath: String){
        mFilePath = filePath
        mBufferSizeInByte = AudioTrack.getMinBufferSize(SAMPLE_RATE, CHANNEL, AUDIO_FORMAT)
        if (mBufferSizeInByte <= 0) {
            throw IllegalArgumentException("AudioTrack is not available $mBufferSizeInByte")
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M){
            mAudioTrack = AudioTrack.Builder()
                .setAudioAttributes(AudioAttributes.Builder()
                    .setUsage(AudioAttributes.USAGE_MEDIA)
                    .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                    .setLegacyStreamType(AudioManager.STREAM_MUSIC)
                    .build())
                .setAudioFormat(AudioFormat.Builder()
                    .setEncoding(AUDIO_FORMAT)
                    .setSampleRate(SAMPLE_RATE)
                    .setChannelMask(CHANNEL)
                    .build())
                .setTransferMode(AudioTrack.MODE_STREAM)
                .setBufferSizeInBytes(mBufferSizeInByte)
                .build()
        } else {
            mAudioTrack = AudioTrack(AudioManager.STREAM_MUSIC, SAMPLE_RATE, CHANNEL, AUDIO_FORMAT,
                mBufferSizeInByte,AudioTrack.MODE_STREAM)
        }
        mStatus = Status.STATUS_READY
    }


    /** 开始播放 */
    @Throws(IllegalArgumentException::class)
    fun start() {
        if (mStatus == Status.STATUS_NOT_READY || mAudioTrack == null) {
            throw IllegalArgumentException("播放器尚未初始化")
        }
        if (mStatus == Status.STATUS_START) {
            throw IllegalArgumentException("正在播放.......")
        }
        mExecutorService.execute {
            runCatching {
                playAudioData()
            }.onFailure {
                Log.e(TAG,it.message.toString())
                mHandler.post {
                    Toast.makeText(mContext,"播放出错",Toast.LENGTH_SHORT).show()
                }
            }
        }
        mStatus = Status.STATUS_START
    }

    private fun playAudioData() {
        var dips: InputStream? = null
        try {
            mHandler.post {
                Toast.makeText(mContext, "开始播放", Toast.LENGTH_SHORT).show()
            }
            dips = DataInputStream(BufferedInputStream(FileInputStream(mFilePath)))

            val bytes = ByteArray(mBufferSizeInByte)
            var length = 0
            //当前播放实例是否初始化成功，如果处于初始化成功的状态并且未播放的状态，那么就调用 play
            if (mAudioTrack != null && mAudioTrack!!.state != AudioTrack.STATE_UNINITIALIZED
                && mAudioTrack!!.playState != AudioTrack.PLAYSTATE_PLAYING
            ) {
                mAudioTrack?.play()
                while (dips.read(bytes)
                        .also { length = it } != -1 && mStatus == Status.STATUS_START
                ) {
                    mAudioTrack?.write(bytes, 0, length)
                }
                mHandler.post {
                    Toast.makeText(mContext, "播放结束", Toast.LENGTH_SHORT).show()
                }
                mStatus = Status.STATUS_STOP
            }

        } catch (e: Exception) {
            e.printStackTrace()
        } finally {
            dips?.close()
        }
    }

    fun stop() {
        Log.d(TAG,"------stop------")
        if (mStatus == Status.STATUS_NOT_READY || mStatus == Status.STATUS_READY) {
            Log.d(TAG,"未开始播放")
        }else{
            mStatus = Status.STATUS_STOP
            mAudioTrack?.stop()
            release()
        }
    }

    private fun release() {
        Log.d(TAG,"-----release-----")
        mStatus = Status.STATUS_NOT_READY
        mAudioTrack?.release()
        mAudioTrack = null

    }


    enum class Status{
        STATUS_NOT_READY,
        STATUS_READY,
        STATUS_START,
        STATUS_STOP
    }

}