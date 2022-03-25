package com.youyi.ndksimple

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Looper
import android.util.Log
import androidx.appcompat.app.AlertDialog
import com.youyi.ndksimple.databinding.ActivityMainBinding
import java.lang.NullPointerException
import java.util.*

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private var count = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        //JNI处理对象
        binding.sampleText.text = getPerson().toString()
        binding.btnGlobalRef.setOnClickListener {
            globalRef()
        }
        binding.btnNativeThread.setOnClickListener {
            testThread()
        }

        // Example of a call to a native method
//        binding.sampleText.text = stringFromJNI()
        //基本数据类型
        test1(true, 'q', 21, 35, 231546831, 3.14f, 3.1415926505, "Peter Parker", 16,
            intArrayOf(1, 2, 3, 4, 5, 6), Person("Peter Parker",16,"male"),
            arrayOf<String>("Spider-Sense","silk","Aunt May","MJ"),
            booleanArrayOf(true,false,true,true,false)
        )

        //动态注册
        dynamicRegister("I'm dynamic!!!")
        //原生利用java代码同步
        nativeThread()
    }

    private fun nativeThread() {
        repeat(10){
            Thread{
                count()
                //native中执行线程同步
                nativeCount()
            }.start()
        }
    }
    //java线程同步
    private fun count(){
        synchronized(this){
            count++
            Log.d("Java", "count: $count")
        }
    }

    fun createDialog() {
        if (Looper.getMainLooper() == Looper.myLooper()) {
            AlertDialog.Builder(this)
                .setTitle("UI DIALOG")
                .setMessage("native Runs On MainThread, Update UI")
                .setPositiveButton("Confirm",null)
                .show()
        } else {
            runOnUiThread {
                AlertDialog.Builder(this)
                    .setTitle("UI DIALOG")
                    .setMessage("native Runs On Child-Thread Switches To MainThread, Update UI")
                    .setPositiveButton("Confirm",null)
                    .show()
            }
        }
    }

    fun testException() {
        throw NullPointerException("MainActivity Throw Exception From Native Test");
    }

    external fun test1(
        boolean: Boolean, c: Char, b: Byte, s: Short, l: Long, f: Float, d: Double, name: String,
        age: Int, i: IntArray, person: Person, str: Array<String>, bArray: BooleanArray
    )

    external fun getPerson(): Person

    /** 动态注册 */
    external fun dynamicRegister(name: String)

    /** 全局引用 */
    external fun globalRef()

    /** 同步 */
    external fun nativeCount()

    external fun uiThread()

    external fun testThread()

    /**
     * A native method that is implemented by the 'ndksimple' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'ndksimple' library on application startup.
        init {
            System.loadLibrary("ndksimple")
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        uiThread()
    }
}