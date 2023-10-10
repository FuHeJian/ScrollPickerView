package com.fhj.mvi_demo

import android.app.Application
import com.airbnb.mvrx.Mavericks

/**
@author: fuhejian
@date: 2023/9/22
 */
class MVIApplication : Application() {
    override fun onCreate() {
        super.onCreate()
        Mavericks.initialize(this)
    }
}