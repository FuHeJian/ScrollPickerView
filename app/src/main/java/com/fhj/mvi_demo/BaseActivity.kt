package com.fhj.mvi_demo

import android.app.ActivityOptions
import android.content.Intent
import android.os.Bundle
import android.os.PersistableBundle
import android.view.ViewGroup
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityOptionsCompat
import androidx.fragment.app.Fragment
import com.airbnb.mvrx.ActivityViewModelContext
import com.airbnb.mvrx.DeliveryMode
import com.airbnb.mvrx.FragmentViewModelContext
import com.airbnb.mvrx.InternalMavericksApi
import com.airbnb.mvrx.Mavericks
import com.airbnb.mvrx.MavericksDelegateProvider
import com.airbnb.mvrx.MavericksState
import com.airbnb.mvrx.MavericksStateFactory
import com.airbnb.mvrx.MavericksView
import com.airbnb.mvrx.MavericksViewModel
import com.airbnb.mvrx.MavericksViewModelConfigFactory
import com.airbnb.mvrx.MavericksViewModelProvider
import com.airbnb.mvrx.RealMavericksStateFactory
import com.airbnb.mvrx._fragmentArgsProvider
import com.airbnb.mvrx._internal
import com.airbnb.mvrx.fragmentViewModel
import com.airbnb.mvrx.lifecycleAwareLazy
import com.airbnb.mvrx.withState
import com.fhj.mvi_demo.databinding.ActivityMainBinding
import com.fhj.mvi_demo.m.MainViewModel
import com.fhj.mvi_demo.state.MainState
import com.fhj.mvi_demo.views.pickscrollviewImpl.StringPickScrollView
import kotlinx.coroutines.CancellableContinuation
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.async
import kotlinx.coroutines.channels.BufferOverflow
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.channels.ProducerScope
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.flow.mapLatest
import kotlinx.coroutines.flow.onEach
import kotlinx.coroutines.flow.reduce
import kotlinx.coroutines.flow.transform
import kotlinx.coroutines.flow.transformLatest
import kotlinx.coroutines.launch
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlinx.coroutines.sync.Semaphore
import kotlinx.coroutines.sync.withPermit
import java.io.File
import kotlin.coroutines.ContinuationInterceptor
import kotlin.coroutines.CoroutineContext
import kotlin.coroutines.EmptyCoroutineContext
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException
import kotlin.coroutines.suspendCoroutine
import kotlin.jvm.functions.FunctionN
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

    operator fun provideDelegate(t: Activity1, property: KProperty<*>) {

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