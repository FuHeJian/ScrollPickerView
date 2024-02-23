package com.fhj.mvi_demo

import android.view.Surface
import kotlin.reflect.KProperty

/**
@author: fuhejian
@date: 2024/1/24
 */
class VideoControler {

    @JvmField
    var playThread = 0L;

    @JvmField
    var seek = 0;

    @JvmField
    var videoPlayInfo = 0L;

    lateinit var Instance: VideoControler
    operator fun getValue(obj: Any, property: KProperty<*>): VideoControler {
        if (!::Instance.isInitialized) {
            Instance = VideoControler()
            return Instance
        } else {
            return Instance
        }
    }

    external fun init()

    external fun loadVideo(surface:Surface,path:String):Int

    external fun parseErrorCode(errorCode:Int):String

    fun postMessage(){

    }

}