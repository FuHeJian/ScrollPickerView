package com.ext.extview

import android.animation.ValueAnimator
import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Camera
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Path
import android.graphics.Rect
import android.graphics.RectF
import android.util.AttributeSet
import android.util.Log
import android.view.GestureDetector
import android.view.Gravity
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.view.animation.AccelerateInterpolator
import android.widget.LinearLayout
import android.widget.TextView
import androidx.coordinatorlayout.widget.ViewGroupUtils
import androidx.core.animation.doOnEnd

/**
@author: fuhejian
@date: 2023/10/16
 */
class SwitchButton : LinearLayout {

    constructor(
        context: Context, attr: AttributeSet?, defStyleAttr: Int, defStyleRes: Int
    ) : super(
        context,
        attr,
        defStyleAttr,
        defStyleRes
    )

    constructor(context: Context, attr: AttributeSet?, defStyleAttr: Int) : this(
        context,
        attr,
        defStyleAttr,
        0
    )

    constructor(context: Context, attr: AttributeSet?) : this(context, attr, 0, 0)
    constructor(context: Context) : this(context, null, 0, 0)

    init {
        this.orientation = HORIZONTAL
        setWillNotDraw(false)
    }

    override fun generateLayoutParams(attrs: AttributeSet?): LayoutParams {
        val layoutParams = super.generateLayoutParams(attrs)
        layoutParams.weight = 1f
        layoutParams.height = ViewGroup.LayoutParams.MATCH_PARENT
        return layoutParams
    }

    var drawRect = RectF()

    @SuppressLint("RestrictedApi")
    override fun onLayout(changed: Boolean, l: Int, t: Int, r: Int, b: Int) {
        super.onLayout(changed, l, t, r, b)
        drawRect.set(
            paddingLeft.toFloat(),
            paddingTop.toFloat(),
            width - paddingRight.toFloat(),
            height - paddingBottom.toFloat()
        )
        clipPath.reset()
        clipPath.addRoundRect(drawRect, roundCornerSize, roundCornerSize, Path.Direction.CW)
        var childAt = getChildAt(0)
        childAt?.let {
            ViewGroupUtils.getDescendantRect(this, it, selectDrawRect)
        }
    }

    override fun addView(child: View?, index: Int, params: ViewGroup.LayoutParams?) {
        check(childCount < 2) { "SwitchButton只允许存在两个子视图" }
        if (child is TextView) {
            child.gravity = Gravity.CENTER
        }
        super.addView(child, index, params)
    }

    var selectColor = Color.parseColor("#0085FF")
    var unSelectColor = Color.parseColor("#4DFFFFFF")
    var roundCornerSize = 25f
    var paint = Paint().apply {
        isAntiAlias = true
    }
    var camera = Camera().apply {

    }

    var selectDrawRect = Rect()
    var childDrawRect = Rect()

    var clipPath = Path()

    var selectPosition = 0

    override fun onDraw(canvas: Canvas) {
        paint.setColor(unSelectColor)
        canvas.clipPath(clipPath)
        canvas.drawColor(unSelectColor)
        paint.setColor(selectColor)
        canvas.save()
        camera.save()
        camera.translate(width / 2f, 0f, 0f)
        camera.rotateY(lastValue.toFloat())
        camera.translate(-width / 2f, 0f, 0f)
        camera.applyToCanvas(canvas)
        camera.restore()
        canvas.drawRect(selectDrawRect, paint)
        canvas.restore()
        super.onDraw(canvas)
    }

    var lastValue = 0
    var offsetAnimator = ValueAnimator().apply {
        duration = 300L
        interpolator = AccelerateInterpolator()
        this.addUpdateListener {
            var value = it.animatedValue as Int
//            selectDrawRect.offset(value - lastValue, 0)
            lastValue = value
            invalidate()
        }
        this.doOnEnd {
            //分发选择的消息
            Log.d(TAG, "offsetAnimator动画结束")
            dispatchListener(selectPosition)
        }
    }

    var TAG = "日志"

    @SuppressLint("RestrictedApi")
    fun selectPosition(position: Int) {
        selectPosition = position
        if (offsetAnimator.isRunning) offsetAnimator.pause()
        offsetAnimator.setIntValues(lastValue, if (selectPosition == 0) 0 else 180)
        offsetAnimator.start()
    }

    private fun dispatchListener(position: Int) {
        listeners.forEach {
            it.onSelected(position)
        }
    }

    val listeners = arrayListOf<Listener>()

    interface Listener {
        fun onSelected(position: Int)
    }

    val eventDelegate = GestureDetector(this.context, object : GestureDetector.OnGestureListener {
        override fun onDown(e: MotionEvent) = true

        override fun onShowPress(e: MotionEvent) {
        }

        override fun onSingleTapUp(e: MotionEvent): Boolean {
            var s = if (selectPosition == 0) 1 else 0
            selectPosition(s)
            return true
        }

        override fun onScroll(
            e1: MotionEvent,
            e2: MotionEvent,
            distanceX: Float,
            distanceY: Float
        ) = false

        override fun onLongPress(e: MotionEvent) {
        }

        override fun onFling(
            e1: MotionEvent,
            e2: MotionEvent,
            velocityX: Float,
            velocityY: Float
        ) = false
    })

    override fun onTouchEvent(event: MotionEvent): Boolean {
        return eventDelegate.onTouchEvent(event)
    }

}