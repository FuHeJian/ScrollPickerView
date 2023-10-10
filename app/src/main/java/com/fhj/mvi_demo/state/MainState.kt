package com.fhj.mvi_demo.state

import android.graphics.drawable.Drawable
import android.view.ViewGroup
import android.widget.ImageView
import com.airbnb.mvrx.MavericksState
import com.airbnb.mvrx.PersistState

/**
@author: fuhejian
@date: 2023/9/21
 */
data class MainState(@PersistState val data: StateData = StateData("hello", 12)) : MavericksState {

}