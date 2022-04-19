package com.youyi.video

import android.content.Context
import android.opengl.GLSurfaceView
import android.os.Handler
import android.os.Looper
import android.util.AttributeSet
import android.view.SurfaceHolder
import android.widget.Toast
import java.util.*


/**
 * des：
 * @author: Muppet
 * @date: 2022/4/19
 */
class YUVPlay : GLSurfaceView,SurfaceHolder.Callback {
    private var mContext: Context = context

    private var yuv420pPath: String? = null
    private var surface: Any? = null

    constructor(context: Context):super(context)

    constructor(context: Context, attrs: AttributeSet):super(context,attrs)

    override fun surfaceCreated(holder: SurfaceHolder) {
        super.surfaceCreated(holder)
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, w: Int, h: Int) {
        super.surfaceChanged(holder, format, w, h)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        super.surfaceDestroyed(holder)
    }

    fun glesPlay(yuv420Path: String, surface: Any) {
        this.yuv420pPath = yuv420Path
        this.surface = surface
        val thread = Thread(playRunnable())
        thread.start()
    }

    external fun nativeGlesPlay(yuv420pPath:String?,surface: Any?)

    external fun nativeWindowPlay(yuv420pPath: String?,surface: Any?)

    external fun onDestroy()

    fun showMessage(message: String) {
        Handler(Looper.getMainLooper()).post {
            Toast.makeText(mContext,message,Toast.LENGTH_SHORT).show()
        }
    }

    private fun playRunnable() = Runnable {
        nativeGlesPlay(yuv420pPath,surface)
    }
    companion object {
        // Used to load the 'video' library on application startup.
        init {
            System.loadLibrary("video")
        }
    }
}