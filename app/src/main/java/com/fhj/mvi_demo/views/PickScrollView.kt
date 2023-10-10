package com.fhj.mvi_demo.views

import android.animation.ValueAnimator
import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.PorterDuff
import android.graphics.PorterDuffXfermode
import android.util.AttributeSet
import android.util.Log
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.View
import android.view.ViewParent
import android.view.animation.Interpolator
import android.widget.OverScroller
import androidx.core.view.doOnLayout
import androidx.core.view.doOnNextLayout
import kotlin.math.absoluteValue

/**
@author: fuhejian
@date: 2023/10/7
 */
abstract class PickScrollView<T> : SimpleView {

    var minMargin = 10f
        set(value) {
            field = value
            invalidate()
        }
    var showItemNum = 3
        set(value) {
            field = value
            invalidate()
        }

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

    var indicatorPosition = 0f;

    /**
     * 初始化默认选择第0个数据项，是否将这个选择事件通知到监听器
     */
    var enableInitializeNotifySelectedItem = true

    var currentPosition = -1

    init {
        this.doOnLayout {
            indicatorPosition = width / 2f
        }
        positionAnimator.addUpdateListener {
            var animatedValue = it.animatedValue
            if (animatedValue is Float) {
                moveLength = animatedValue
                invalidate()
            }
        }
        mPaint = getPorterDuffXferPaint()
        mMaskingPaint.isAntiAlias = true
        mMaskingPaint.color = Color.GREEN
        if (enableInitializeNotifySelectedItem) {
            selectPosition(0, false)
        }
    }

    /**
     * 要绘制的内容
     */
    fun getValue(position: Int) = data.get(position)

    /**
     * 返回绘制的宽度
     */
    abstract fun getValueWidth(position: Int): Float

    abstract fun getValueHeight(position: Int): Float

    /**
     * item的大小
     */
    private fun getValueSize() = data.size

    abstract fun drawItem(
        canvas: Canvas,
        item: T,
        offset: Float,
        position: Int,
        offsetSelectedIndex: Int
    )

    var mMaskingWidth = 0f

    var enableMasking = false
        set(value) {
            field = value
            invalidate()
        }

    val dataPosition = HashMap<Int, Float>()

    var data = emptyList<T>()
        set(value) {
            field = value
            moveLengthMax = 1f
            dataPosition.clear()
            currentPosition = -1
            if (isLaidOut) {
                invalidate()
            }
        }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        val size = getValueSize()
        var sumWidth = 0f
        var w = width
        val isRefreshDataPosition = dataPosition.isEmpty()

        if (moveLengthMax == 1f) {
            for (i in 0 until size) {
                val itemW = getValueWidth(i)
                var margin = 0f
                if (i == size - 1) {
                    margin = 0f
                } else {
                    margin = getItemMargin(w * 1f, itemW)
                }
                if (i == 0) {
                    moveLengthMin = -((w / 2 - itemW / 2).coerceAtLeast(0f))
                    moveLength = moveLengthMin
                }
                if (isRefreshDataPosition) {
                    dataPosition.put(i, sumWidth + itemW / 2f)
                }
                sumWidth += itemW + margin
                if (i == size - 1) {
                    moveLengthMax = sumWidth - w + ((w / 2 - itemW / 2).coerceAtLeast(0f))
                }
            }
            val sumW = moveLengthMax + moveLengthMin.absoluteValue + width
            mMaskBitmap = Bitmap.createBitmap(sumW.toInt(), height, Bitmap.Config.ARGB_8888)
            mMaskBitmap?.let {
                mMaskCanvas = Canvas(it)
                mMaskingPaint?.let {
                    mMaskCanvas?.let {
                        it.drawColor(Color.TRANSPARENT)
                        it.drawRect(width / 4f, 0f, 3 * width / 4f, height * 1f, mMaskingPaint)
                    }

                }
            }
        }

        restrictMoveLength()
        canvas.save()
        if (orientation == ORIENTATION_HORIZONTAL) {//moveLength永远大于0
            canvas.translate(-moveLength, 0f)
        } else {
            canvas.translate(0f, -moveLength)
        }

        sumWidth = 0f

        for (i in 0 until size) {
            val itemW = getValueWidth(i)
            var margin = 0f
            if (i == size - 1) {
                margin = 0f
            } else {
                margin = getItemMargin(w * 1f, itemW)
            }
            if (sumWidth - moveLength > width) continue
            sumWidth += itemW + margin
            if (sumWidth < moveLength) {

            } else {
                getNearIndicatorPosition()?.let {
                    val offset =
                        (dataPosition.get(i) ?: 0f) - moveLength -
                                indicatorPosition
                    drawItem(
                        canvas,
                        getValue(i),
                        offset.absoluteValue / indicatorPosition,
                        i,
                        i - it.key
                    )
                }
            }
            if (orientation == ORIENTATION_HORIZONTAL) {
                canvas.translate(getValueWidth(i) + margin, 0f)
            } else {
                canvas.translate(0f, getValueHeight(i) + margin)
            }
        }
        canvas.restore()
//        val p = canvas.saveLayer(width / 4f, 0f, 3 * width / 4f, height * 1f, null)
        if (enableMasking) {
            var drawMasking = drawMasking(moveLengthMax + moveLengthMin.absoluteValue + width)
            canvas.drawBitmap(drawMasking!!, 0f, 0f, mPaint)
        }
//        canvas.restoreToCount(p)
    }

    fun drawMasking(sumW: Float): Bitmap? {
        /*mMaskCanvas?.apply {
            save()
            translate(moveLength, 0f)
            restore()
        }*/
        return mMaskBitmap
    }

    fun restrictMoveLength() {
        moveLength = moveLength.coerceIn(moveLengthMin - overScroll, moveLengthMax + overScroll)
    }

    fun getItemMargin(w: Float, _itemWidth: Float): Float {
        if (orientation == ORIENTATION_HORIZONTAL) {
            val _w: Float = (w - _itemWidth) / (showItemNum - 1)
            val margin = _w - _itemWidth
            if (margin > minMargin) return margin else return minMargin
        } else {
            return minMargin
        }
    }

    val flingDelegate: OverScroller = OverScroller(context, object : Interpolator {
        override fun getInterpolation(it: Float): Float {
            var t = it
            t -= 1.0f
            return t * t * t * t * t + 1.0f
        }
    })

    val scrollDelegate = GestureDetector(context, object : GestureDetector.OnGestureListener {
        override fun onDown(p0: MotionEvent): Boolean {
            flingDelegate.abortAnimation()
            currentScrollOrientation = 0;
            return true
        }


        override fun onShowPress(p0: MotionEvent) {

        }

        override fun onSingleTapUp(p0: MotionEvent): Boolean {
            autoSelectedNearPosition()
            return true
        }

        override fun onScroll(p0: MotionEvent?, p1: MotionEvent, sX: Float, sY: Float): Boolean {
            if (currentScrollOrientation == 0) {
                currentScrollOrientation =
                    if (Math.abs(sX) >= Math.abs(sY)) ORIENTATION_HORIZONTAL else ORIENTATION_VERTICAL
            }
            if (Math.abs(sX) >= Math.abs(sY) && orientation == ORIENTATION_HORIZONTAL) {//水平
                if (parent is ViewParent) {
                    parent.requestDisallowInterceptTouchEvent(true)
                }
                moveLength += sX
                invalidate()
                return true;
            } else if (Math.abs(sX) < Math.abs(sY) && orientation == ORIENTATION_VERTICAL) {//竖直
                moveLength += sY
                invalidate()
                return true;
            }
            parent.requestDisallowInterceptTouchEvent(false)
            return false;
        }

        override fun onLongPress(p0: MotionEvent) {

        }

        override fun onFling(
            p0: MotionEvent?,
            p1: MotionEvent,
            velocityX: Float,
            velocityY: Float
        ): Boolean {
            Log.d("日志", "onFling: ")
            lastFlingCurrentXY = 0
            positionAnimator.cancel()
            flingStarted = true
            flingDelegate.fling(
                0,
                0,
                velocityX.toInt(),
                velocityY.toInt(),
                -moveLengthMax.toInt(),
                moveLengthMax.toInt(),
                -moveLengthMax.toInt(),
                moveLengthMax.toInt(), 0, 0
            )
            invalidate()
            return true;
        }
    })

    var moveLength: Float = 0f
    var moveLengthMax: Float = 1f
    var moveLengthMin: Float = 0f

    var overScroll: Float = 100f

    var lastFlingCurrentXY = 0
    var flingStarted = false;

    override fun computeScroll() {
        super.computeScroll()
        if (flingDelegate.computeScrollOffset()) {
            val x = flingDelegate.currX
            val y = flingDelegate.currY
            if (orientation == ORIENTATION_HORIZONTAL) {
                moveLength += (lastFlingCurrentXY - x)
                lastFlingCurrentXY = x
            } else {
                moveLength += (lastFlingCurrentXY - y)
                lastFlingCurrentXY = y
            }
            invalidate()
        } else {

            if (flingStarted) {
                flingStarted = false
                autoSelectedNearPosition()
            }

        }
    }

    companion object {
        const val ORIENTATION_HORIZONTAL = 1;
        const val ORIENTATION_VERTICAL = 2;
        val positionAnimator = ValueAnimator()
        var mMaskingPaint = Paint()
        var mMaskBitmap: Bitmap? = null
        var mMaskCanvas: Canvas? = null
    }

    var mPaint: Paint? = null

    private fun getPorterDuffXferPaint(): Paint {
        var mPaint = Paint()
        var porterDuffXfermode = PorterDuffXfermode(PorterDuff.Mode.SCREEN)
        mPaint.isAntiAlias = true
        mPaint.xfermode = porterDuffXfermode
        mPaint.color = Color.GREEN
        mPaint.style = Paint.Style.FILL
        return mPaint
    }

    var orientation: Int = ORIENTATION_HORIZONTAL;
    var currentScrollOrientation: Int = 0;

    override fun onTouchEvent(event: MotionEvent?): Boolean {
        if (event != null) {

            when (event.actionMasked) {
                MotionEvent.ACTION_CANCEL -> {
                    autoSelectedNearPosition()
                }
            }
            var handled = scrollDelegate.onTouchEvent(event);
            if (event.actionMasked == MotionEvent.ACTION_UP) {
                if (!handled) {
                    autoSelectedNearPosition()
                    return true
                }
            }
            return handled
        } else {
            autoSelectedNearPosition()
        }
        return false;
    }

    private fun animateToPosition(position: Int) {
        var item = dataPosition.get(position)
        item?.let {
            val target = it - indicatorPosition
            if (positionAnimator.isRunning) {
                positionAnimator.cancel()
            }
            positionAnimator.duration = 300
            positionAnimator.setFloatValues(moveLength, target)
            positionAnimator.start()
        }
    }

    fun selectPosition(position: Int, animate: Boolean) {

        doOnDrawLater {
            if (currentPosition != position) {
                mListener?.let {
                    it.onSelect(position, getValue(position))
                }
                currentPosition = position
            }
            if (animate) {
                animateToPosition(position)
            } else {
                var item = dataPosition.get(position)
                item?.let {
                    moveLength = it - indicatorPosition
                    invalidate()
                }
            }
        }
        if (isLaidOut) {
            invalidate()
        }
    }

    private fun autoSelectedNearPosition() {
        if (dataPosition.isEmpty()) return
        var min = getNearIndicatorPosition()
        min?.let {
            selectPosition(it.key, true)
        }
    }

    private fun getNearIndicatorPosition() = dataPosition.minBy {
        (it.value - (moveLength + indicatorPosition)).absoluteValue
    }

    var mListener: SelectListener<T>? = null

    interface SelectListener<T> {
        fun onSelect(position: Int, item: T);
    }

}