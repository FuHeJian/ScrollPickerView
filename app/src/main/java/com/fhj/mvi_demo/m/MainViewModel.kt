package com.fhj.mvi_demo.m

import com.airbnb.mvrx.MavericksViewModel
import com.fhj.mvi_demo.BaseActivity
import com.fhj.mvi_demo.state.MainState
import com.fhj.mvi_demo.state.StateData
import kotlin.reflect.KProperty

/**
@author: fuhejian
@date: 2023/9/21
 */
class MainViewModel : MavericksViewModel<MainState> {



    operator fun getValue(baseActivity: BaseActivity, property: KProperty<*>): MainViewModel {
        println("getValue调用")
        return this;
    }



    var k = 1;

    constructor(
        initialState: MainState
    ) : super(initialState) {

    }

    fun setK() {
        k++;
        update();
    }

    fun update() {
        setState {
            this.copy(StateData(k.toString(), k))
        }
    }

}