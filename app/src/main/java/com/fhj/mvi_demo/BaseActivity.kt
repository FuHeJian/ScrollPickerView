package com.fhj.mvi_demo

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.airbnb.mvrx.ActivityViewModelContext
import com.airbnb.mvrx.InternalMavericksApi
import com.airbnb.mvrx.Mavericks
import com.airbnb.mvrx.MavericksDelegateProvider
import com.airbnb.mvrx.MavericksState
import com.airbnb.mvrx.MavericksStateFactory
import com.airbnb.mvrx.MavericksView
import com.airbnb.mvrx.MavericksViewModel
import com.airbnb.mvrx.MavericksViewModelProvider
import com.airbnb.mvrx.RealMavericksStateFactory
import com.airbnb.mvrx._internal
import com.airbnb.mvrx.lifecycleAwareLazy
import com.airbnb.mvrx.withState
import com.fhj.mvi_demo.databinding.ActivityMainBinding
import com.fhj.mvi_demo.m.MainViewModel
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlin.reflect.KClass
import kotlin.reflect.KProperty

/**
@author: fuhejian
@date: 2023/9/21
 */
private val refun = BaseActivity::em as Function2<BaseActivity, String, Any>

open class BaseActivity : AppCompatActivity(), MavericksView {

    init {
        System.loadLibrary("test")
    }

    private val viewModel by fragmentViewModel(MainViewModel::class)

    @OptIn(InternalMavericksApi::class)
    inline fun <T, reified VM : MavericksViewModel<S>, reified S : MavericksState> T.fragmentViewModel(
        viewModelClass: KClass<VM> = VM::class,
        crossinline keyFactory: () -> String = { viewModelClass.java.name }
    ): MavericksDelegateProvider<T, VM> where T : MavericksView =
        viewModelDelegateProvider<T, VM, S>(
            viewModelClass,
            keyFactory,
            existingViewModel = false
        ) { stateFactory ->
            MavericksViewModelProvider.get(
                viewModelClass = viewModelClass.java,
                stateClass = S::class.java,
                viewModelContext = ActivityViewModelContext(
                    activity = this@BaseActivity,
                    args = this@BaseActivity.intent.extras?.get(Mavericks.KEY_ARG),
                ),
                key = keyFactory(),
                initialStateFactory = stateFactory
            )
        }

    @OptIn(InternalMavericksApi::class)
    inline fun <T, reified VM : MavericksViewModel<S>, reified S : MavericksState> viewModelDelegateProvider(
        viewModelClass: KClass<VM>,
        crossinline keyFactory: () -> String,
        existingViewModel: Boolean,
        noinline viewModelProvider: (stateFactory: MavericksStateFactory<VM, S>) -> VM
    ): MavericksDelegateProvider<T, VM> where T : MavericksView =
        object : MavericksDelegateProvider<T, VM>() {

            override operator fun provideDelegate(
                thisRef: T,
                property: KProperty<*>
            ): Lazy<VM> {
                return lifecycleAwareLazy(thisRef) {
                    viewModelProvider(RealMavericksStateFactory())
                        .apply { _internal(thisRef, action = { thisRef.postInvalidate() }) }
                }
            }

        }

    //状态需要更新时
    override fun invalidate() {
        withState(viewModel) {

        }
    }

    lateinit var inflate: ActivityMainBinding;
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        inflate = ActivityMainBinding.inflate(layoutInflater)

        setContentView(inflate.root)

        var coroutineScope = CoroutineScope(Dispatchers.IO)

        coroutineScope.launch {
            getRe()
        }

        val p = refun(this, "")

    }


    fun getRe() = "hello"

    override fun onResume() {
        super.onResume()
    }

    fun em(s: String) {

    }

    external fun op(): Int;

}