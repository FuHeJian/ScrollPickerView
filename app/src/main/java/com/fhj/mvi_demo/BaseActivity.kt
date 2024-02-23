package com.fhj.mvi_demo

import android.Manifest
import android.annotation.SuppressLint
import android.content.Intent
import android.graphics.Color
import android.os.Bundle
import android.provider.MediaStore
import android.util.Log
import android.view.Surface
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.core.Camera
import androidx.core.app.ActivityCompat
import androidx.core.database.getStringOrNull
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
import kotlin.random.Random
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

    private val videoController by VideoControler()

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
            ): Lazy<VM> {//by的初始化工作
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
    lateinit var cameraX: Camera

    override fun onRestart() {
        super.onRestart()
        Log.d(TAG, "1-onRestart: ")
    }

    override fun onStart() {
        super.onStart()
        Log.d(TAG, "1-onStart: ")
    }

    override fun onRestoreInstanceState(savedInstanceState: Bundle) {
        super.onRestoreInstanceState(savedInstanceState)
    }

    override fun onResume() {
        super.onResume()
        Log.d(TAG, "1-onResume: ")
    }

    @SuppressLint("RestrictedApi")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        Log.d(TAG, "1-onCreate: ")

        inflate = ActivityMainBinding.inflate(layoutInflater)

        ActivityCompat.requestPermissions(
            this,
            arrayOf(
                Manifest.permission.READ_MEDIA_VIDEO,
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.READ_MEDIA_IMAGES,
                Manifest.permission.ACCESS_MEDIA_LOCATION,
                Manifest.permission.CAMERA
            ), 101)

        setContentView(inflate.root)
/*        var instance = ProcessCameraProvider.getInstance(this)
        instance.addListener({
            var camera = instance.get()
            *//*            var useCasePreview = Preview.Builder().build().also {
                            it.setSurfaceProvider(inflate.preview.surfaceProvider)
                        }*//*
//            cameraX =
//                camera.bindToLifecycle(this, CameraSelector.DEFAULT_BACK_CAMERA, useCasePreview)
        }, ContextCompat.getMainExecutor(this))

        var coroutineScope = CoroutineScope(Dispatchers.Main)

        *//*inflate.button.setOnClickListener {
                    coroutineScope.launch {
                        println("结果" + getRe())
                    }
        }*//*

        inflate.Carousel.setAdapter(object : Carousel.Adapter {

            override fun count() = 5

            override fun populate(view: View?, index: Int) {
                Log.d(TAG, "populate index = " + index)
                view?.setBackgroundColor(getViewColor(index))
            }

            override fun onNewItem(index: Int) {
                Log.d(TAG, "onNewItem index = " + index)
                *//*              referenceList.forEach{
                                  Log.d(TAG, "referenceList: 获取值_列表 " + it + ",value = " + it.get())
                              }*//*
            }
        })
        videoController.init();
        inflate.TODO.setOnClickListener{
            CoroutineScope(Dispatchers.IO).launch {
                var re = videoController.loadVideo(inflate.surfaceView.holder.surface,getAVideoPath())
                withContext(Dispatchers.Main){
                    Toast.makeText(this@BaseActivity, videoController.parseErrorCode(re), Toast.LENGTH_SHORT).show()
                }

            }
        }*/
        inflate.TODO.extend()

        inflate.openFile.setOnClickListener{
            /*val file = File(getAVideoPath())
            file.readBytes()*/
            startActivity(Intent(this,SecondActivity::class.java))
        }

    }

    val TAG = "日志"

    fun getViewColor(position: Int): Int {
        when (position) {
            0 -> return Color.RED
            1 -> return Color.BLUE
            2 -> return Color.GREEN
            3 -> return Color.CYAN
            4 -> return Color.YELLOW
            else -> return Color.BLACK
        }
    }

    //fun getRe() = op(getAVideoPath(),inflate.preview.holder.surface);
    fun getRe() = 1

    fun em(s: String) {

    }

    /**
     * 随机获取一个视频
     */
    fun getAVideoPath(): String {
//        var videoUri = MediaStore.getMediaUri(this, MediaStore.Video.Media.EXTERNAL_CONTENT_URI)

        MediaStore.Video.Media.EXTERNAL_CONTENT_URI?.let {
            var query = contentResolver.query(
                it,
                arrayOf(MediaStore.Video.VideoColumns.DATA),
                null,
                null,
                MediaStore.Video.DEFAULT_SORT_ORDER
            )
            query?.let {
                if (it.count > 0) {
                    var random = Random(System.currentTimeMillis()).nextInt(it.count)
                    it.moveToPosition(random)
                    var columnCount = it.columnCount
                    for (i in columnCount / 2 until columnCount) {
                        var s = it.getStringOrNull(i) ?: ""
                        println(it.getColumnName(i) + "==结果==" + s)
                        return s;
                    }
                }
            }
        }

        return ""
    }

    external fun op(p: String, surface: Surface?): Int

    override fun onStop() {
        super.onStop()
        Log.d(TAG, "1-onStop: ")
    }

    override fun onPause() {
        super.onPause()
        Log.d(TAG, "1-onPause: ")
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.d(TAG, "1-onDestroy: ")
    }
}