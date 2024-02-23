package com.ext.extview

import android.animation.ObjectAnimator
import android.graphics.Rect
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.View
import android.view.animation.OvershootInterpolator
import androidx.core.view.doOnAttach
import androidx.core.view.doOnDetach
import com.ext.extview.utils.minus
import kotlin.math.absoluteValue

/**
@author: fuhejian
@date: 2023/11/8
 */
class SuctionEdge() {

    var rect: Rect = Rect()
        set(value) {
            field = value

            field.let {
                EDGE.LEFT_BOTTOM.value_1 = it.bottom
                EDGE.LEFT_BOTTOM.value_2 = it.left
                EDGE.LEFT_TOP.value_1 = it.top
                EDGE.LEFT_TOP.value_2 = it.left
                EDGE.RIGHT_TOP.value_1 = it.top
                EDGE.RIGHT_TOP.value_2 = it.right
                EDGE.RIGHT_BOTTOM.value_1 = it.bottom
                EDGE.RIGHT_BOTTOM.value_2 = it.right
            }
        }

    var suctionView: View? = null
        set(value) {
            field = value
            field?.let {
                delegate = GestureDetector(field!!.context, touchListener)
                animatorX.target = suctionView
                animatorY.target = suctionView
            }
            if (field == null) {
                delegate = null
            }
            suctionView?.doOnAttach {
                suctionView?.doOnDetach {
                    suctionView = null
                    animatorX.cancel()
                    animatorY.cancel()
                }
            }
        }

    val animatorX = ObjectAnimator()
    val animatorY = ObjectAnimator()
    val inter = OvershootInterpolator()
    init {
        animatorX.setPropertyName("translationX")
        animatorY.setPropertyName("translationY")
        animatorX.duration  =300L
        animatorY.duration  =300L
        animatorX.interpolator = inter
        animatorY.interpolator = inter
    }

    private var delegate: GestureDetector? = null
    private var lastX = 0f
    private var lastY = 0f

    private var touchListener = object : GestureDetector.SimpleOnGestureListener() {

        override fun onScroll(
            e1: MotionEvent,
            e2: MotionEvent,
            distanceX: Float,
            distanceY: Float
        ): Boolean {

            val dx = (e2?.getRawX()?:0f)  -  (e1?.getRawX()?:0f)
            val dy = (e2?.getRawY()?:0f)  -  (e1?.getRawY()?:0f)

            suctionView?.translationX  = (suctionView?.translationX?:0f) + dx - lastX
            suctionView?.translationY = (suctionView?.translationY?:0f) + dy - lastY

            lastX = dx
            lastY = dy

            return true;
        }

        override fun onDown(e: MotionEvent): Boolean {
            lastX = 0f
            lastY = 0f
            return true
        }

    }

    fun process(event: MotionEvent): Boolean {

        val re = delegate?.onTouchEvent(event) ?: true

        when(event.actionMasked){
            MotionEvent.ACTION_UP->{
                moveToEdge()
            }
        }

        return re

    }

    enum class EDGE {
        LEFT_TOP,
        LEFT_BOTTOM,
        RIGHT_TOP,
        RIGHT_BOTTOM;

        var value_1 = 0;
        var value_2 = 0;

        constructor()
        constructor(_v1: Int, _v2: Int) {
            value_1 = _v1
            value_2 = _v2
        }
    }

    fun getEdge(): EDGE {
        return suctionView?.let {
            val left = it.x
            val top = it.y
            EDGE.values().minBy {
                (left - it.value_2).absoluteValue + (top - it.value_1).absoluteValue
            }
        } ?: EDGE.LEFT_TOP
    }

    var ani = true

    var rectCache = Rect()
    var rectCache2 = Rect()
    fun moveToEdge() {
        if (suctionView == null) return
        val edge = getEdge()
        var moveLengthX = 0
        var moveLengthY = 0
        suctionView?.getHitRect(rectCache)
        rectCache2.set(rect - rectCache)

        when (edge) {
            EDGE.LEFT_TOP -> {
                moveLengthX = rectCache2.left
                moveLengthY = rectCache2.top
            }

            EDGE.LEFT_BOTTOM -> {
                moveLengthX = rectCache2.left
                moveLengthY = rectCache2.bottom
            }

            EDGE.RIGHT_TOP -> {
                moveLengthX = rectCache2.right
                moveLengthY = rectCache2.top
            }

            EDGE.RIGHT_BOTTOM -> {
                moveLengthX = rectCache2.right
                moveLengthY = rectCache2.bottom
            }
        }

        val x = (suctionView?.translationX?:0f)
        val y = (suctionView?.translationY?:0f)

        val dx = x + moveLengthX * 1f
        val dy = y + moveLengthY * 1f

        if(ani){
            if(animatorX.isRunning){
                animatorX.cancel()
            }
            if(animatorY.isRunning){
                animatorY.cancel()
            }
            animatorX.setFloatValues(x,dx)
            animatorY.setFloatValues(y,dy)
            animatorX.start()
            animatorY.start()
        }else{
            suctionView?.translationX = dx
            suctionView?.translationY = dy
        }

    }


}