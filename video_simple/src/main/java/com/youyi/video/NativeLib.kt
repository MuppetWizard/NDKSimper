package com.youyi.video

import android.graphics.YuvImage

class NativeLib {


    /**
     * A native method that is implemented by the 'video' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'video' library on application startup.
        init {
            System.loadLibrary("video")
        }
    }
}