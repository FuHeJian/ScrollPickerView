package com.fhj.mvi_demo.state

import com.airbnb.mvrx.MavericksState
import com.airbnb.mvrx.PersistState

/**
@author: fuhejian
@date: 2023/9/21
 */
data class MainState(@PersistState val data: StateData = StateData("hello", 12)) : MavericksState {

}