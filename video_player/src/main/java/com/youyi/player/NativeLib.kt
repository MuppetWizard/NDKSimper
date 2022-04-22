package com.youyi.player

import android.view.Surface

class NativeLib {

    /**
     * A native method that is implemented by the 'player' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    /** 当前ffmpeg版本 */
    external fun getFFmpegVersion(): String

    /** 设置 surface
     * @param surface
     */
    external fun setSurfaceNative(surface: Surface)

    /** 准备工作
     * @param mDataSource 播放资源
     */
    external fun prepareNative(mDataSource: String)

    /** 准备完成 开始播放 */
    external fun startNative()

    /** 点击停止播放和，调用该函数恢复播放 */
    external fun reStartNative()

    /** 停止播放 */
    external fun stopNative()

    /** 释放资源 */
    external fun releaseNative()

    /** 是否正在播放 */
    external fun isPlayNative()

    companion object {
        init {
            System.loadLibrary("PLAYER")
        }
    }
}