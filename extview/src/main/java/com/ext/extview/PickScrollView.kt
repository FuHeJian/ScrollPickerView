package com.ext.extview

import android.animation.ValueAnimator
import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.PorterDuff
import android.graphics.PorterDuffXfermode
import android.util.AttributeSet
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.ViewParent
import android.view.animation.Interpolator
import android.widget.OverScroller
import androidx.core.view.doOnLayout
import kotlin.math.absoluteValue

/**
@author: fuhejian
@date: 2023/10/7
 */
abstract class PickScrollView<T> : SimpleView {

    companion object {
        const val ORIENTATION_HORIZONTAL = 1;
        const val ORIENTATION_VERTICAL = 2;
    }

    var minMargin = 10f
        set(value) {
            field = value
            resetPosition()
        }
    var showItemNum = 3f
        set(value) {
            field = value
            resetPosition()
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

    var orientation: Int = ORIENTATION_HORIZONTAL
        set(value) {
            field = value
            resetPosition()
        }
    private var currentScrollOrientation: Int = 0;

    /**
     * 要绘制的内容
     */
    fun getSelectedItem(position: Int) = data.get(position)

    /**
     * 返回绘制该Item的宽度，只会在水平滑动时调用
     */
    abstract fun getValueWidth(position: Int): Float

    /**
     * 返回绘制该Item的高度，只会在竖直滑动时调用
     */
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
            resetPosition()
        }

    private fun resetPosition() {
        moveLengthMax = 1f
        currentPosition = -1
        dataPosition.clear()
        if (isLaidOut) {
            invalidate()
        }
    }

    private var lastSelectPosition = 0
    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        val size = getValueSize()
        var sumWidth = 0f
        var w = if (orientation == ORIENTATION_HORIZONTAL) width else height
        val isRefreshDataPosition = dataPosition.isEmpty()

        if (moveLengthMax == 1f) {
            indicatorPosition =
                if (orientation == ORIENTATION_HORIZONTAL) width / 2f else height / 2f
            for (i in 0 until size) {
                val itemW =
                    if (orientation == ORIENTATION_HORIZONTAL) getValueWidth(i) else getValueHeight(
                        i
                    )
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
                    dataPosition.put(i, sumWidth + (itemW / 2f))
                }
                sumWidth += itemW + margin
                if (i == size - 1) {
                    moveLengthMax =
                        (sumWidth - w + ((w / 2 - itemW / 2).coerceAtLeast(0f))).coerceAtLeast(
                            moveLengthMin
                        )
                }
            }
            val sumW = moveLengthMax + moveLengthMin.absoluteValue + w
            mMaskBitmap?.recycle()
            mMaskBitmap = Bitmap.createBitmap(sumW.toInt(), height, Bitmap.Config.ARGB_8888)
            mMaskBitmap?.let {
                mMaskCanvas = Canvas(it)
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

        var nearIndicatorPosition = getNearIndicatorPosition()?.key ?: 0
        var offset_left = 0f
        var offset_right = 0f
        var offset = 0f

        lastSelectPosition = nearIndicatorPosition
        lastSelectPosition.let {
            offset =
                (dataPosition.get(it) ?: 0f) - moveLength -
                        indicatorPosition

            if (offset > 0 && dataPosition.contains(it - 1)) {
                offset_left =
                    offset.absoluteValue / ((dataPosition.get(it - 1) ?: 0f)
                            - (dataPosition.get(it) ?: 0f))
            }

            if (offset < 0 && dataPosition.contains(it + 1)) {
                offset_right =
                    offset.absoluteValue / ((dataPosition.get(it + 1)
                        ?: 0f) - (dataPosition.get(it) ?: 0f))
            }
        }

        for (i in 0 until size) {
            val itemW =
                if (orientation == ORIENTATION_HORIZONTAL) getValueWidth(i) else getValueHeight(i)
            var margin = 0f
            if (i == size - 1) {
                margin = 0f
            } else {
                margin = getItemMargin(w * 1f, itemW)
            }
            if (sumWidth - moveLength > w) continue
            sumWidth += itemW + margin
            if (sumWidth < moveLength) {
//                Log.d("日志", "onDraw: ")
            } else {
                lastSelectPosition.let {

                    drawItem(
                        canvas,
                        getSelectedItem(i),
                        if (offset < 0) offset_right else offset_left,
                        i,
                        i - it
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
            var drawMasking = drawMasking(nearIndicatorPosition)
            canvas.drawBitmap(drawMasking!!, 0f, 0f, mPaint)
        }
//        canvas.restoreToCount(p)
    }

    fun getMakingShowItemNumber() = 3;//显示中间3个

    var mask_start_position = 0f
    var mask_end_position = 0f
    var textColor = Color.parseColor("#FFffffff")
    fun drawMasking(nearIndicatorPosition: Int): Bitmap? {
        var minusValue = (getMakingShowItemNumber() - 1) / 2
        mMaskCanvas?.apply {
            save()
            drawColor(textColor, PorterDuff.Mode.SRC)
            mMaskingPaint?.let {
                mMaskCanvas?.let {

                    if (orientation == ORIENTATION_HORIZONTAL) {

                        if (dataPosition.contains(nearIndicatorPosition - minusValue)) {
                            mask_start_position =
                                dataPosition.get(nearIndicatorPosition - minusValue)
                                    ?.minus(getValueWidth(nearIndicatorPosition - minusValue).div(2f))
                                    ?: 0f
                        } else {
                            mask_start_position = dataPosition.get(nearIndicatorPosition)
                                ?.minus(getValueWidth(nearIndicatorPosition).div(2f)) ?: 0f
                        }

                        if (dataPosition.contains(nearIndicatorPosition + minusValue)) {
                            mask_end_position = dataPosition.get(nearIndicatorPosition + minusValue)
                                ?.plus(getValueWidth(nearIndicatorPosition + minusValue).div(2f))
                                ?: (width * 1f)
                        } else {
                            mask_end_position = dataPosition.get(nearIndicatorPosition)
                                ?.plus(getValueWidth(nearIndicatorPosition).div(2f)) ?: (width * 1f)
                        }

                        mask_start_position = mask_start_position - moveLength
                        mask_end_position = mask_end_position - moveLength

                        it.drawRect(
                            mask_start_position,
                            0f,
                            mask_end_position,
                            height * 1f,
                            mMaskingPaint
                        )

                    } else {

                        if (dataPosition.contains(nearIndicatorPosition - 1)) {
                            mask_start_position = dataPosition.get(nearIndicatorPosition - 1)
                                ?.minus(getValueHeight(nearIndicatorPosition - 1).div(2f)) ?: 0f
                        } else {
                            mask_start_position = dataPosition.get(nearIndicatorPosition)
                                ?.minus(getValueHeight(nearIndicatorPosition).div(2f)) ?: 0f
                        }

                        if (dataPosition.contains(nearIndicatorPosition + 1)) {
                            mask_end_position = dataPosition.get(nearIndicatorPosition + 1)
                                ?.plus(getValueHeight(nearIndicatorPosition + 1).div(2f))
                                ?: (width * 1f)
                        } else {
                            mask_end_position = dataPosition.get(nearIndicatorPosition)
                                ?.plus(getValueHeight(nearIndicatorPosition).div(2f))
                                ?: (width * 1f)
                        }
                        it.drawRect(
                            0f,
                            mask_start_position,
                            width * 1f,
                            mask_end_position,
                            mMaskingPaint
                        )
                    }
                }
            }
            restore()
        }
        return mMaskBitmap
    }

    fun restrictMoveLength() {
        moveLength = moveLength.coerceIn(moveLengthMin - overScroll, moveLengthMax + overScroll)
    }

    fun getItemMargin(w: Float, _itemWidth: Float): Float {
        val _w: Float = (w - _itemWidth) / (showItemNum - 1)
        val margin = _w - _itemWidth
        if (margin > minMargin) return margin else return minMargin
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

        override fun onScroll(p0: MotionEvent, p1: MotionEvent, sX: Float, sY: Float): Boolean {
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
            p0: MotionEvent,
            p1: MotionEvent,
            velocityX: Float,
            velocityY: Float
        ): Boolean {
            lastFlingCurrentXY = 0
            positionAnimator.cancel()
            flingStarted = true
            var min = Int.MIN_VALUE
            var max = Int.MAX_VALUE

            flingDelegate.fling(
                0,
                0,
                velocityX.toInt() / 2,
                velocityY.toInt() / 2,
                min,
                max,
                min,
                max, 0, 0
            )
            invalidate()
            return true;
        }
    })

    var moveLength: Float = 0f
    var moveLengthMax: Float = 1f
    var moveLengthMin: Float = 0f

    /**
     * 设置滑动时可以超出边界的距离
     */
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


    var mPaint: Paint? = null

    private fun getPorterDuffXferPaint(): Paint {
        var mPaint = Paint()
        var porterDuffXfermode = PorterDuffXfermode(PorterDuff.Mode.MULTIPLY)
        mPaint.isAntiAlias = true
        mPaint.xfermode = porterDuffXfermode
        mPaint.color = Color.RED
        mPaint.style = Paint.Style.FILL
        return mPaint
    }

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

    fun setSelectedPosition(position: Int) {
        selectPosition(position, false)
    }

    fun setSelectedPositionNoInformLisenter(position: Int) {
        currentPosition = position
        selectPosition(position, false)
    }

    fun selectPosition(position: Int, animate: Boolean) {

        if (data.isEmpty() || position < 0 || position >= data.size) return
        doOnDrawLater {
            if (currentPosition != position) {
                currentPosition = position
                mListener?.let {
                    it.onSelect(this, position, getSelectedItem(position))
                }
                lastSelectPosition = position
            }
            if (animate) {
                animateToPosition(position)
            } else {
                if (positionAnimator.isRunning) {
                    positionAnimator.cancel()
                }
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
        fun onSelect(pickview: PickScrollView<T>, position: Int, item: T);
    }

    val positionAnimator = ValueAnimator()
    var mMaskingPaint = Paint()
    var mMaskBitmap: Bitmap? = null
    var mMaskCanvas: Canvas? = null

    init {
        this.doOnLayout {
            indicatorPosition =
                if (orientation == ORIENTATION_HORIZONTAL) width / 2f else height / 2f
        }
        positionAnimator.addUpdateListener {
            var animatedValue = it.animatedValue
            if (animatedValue is Float) {
                moveLength = animatedValue
                invalidate()
            }
        }
        /*        positionAnimator.doOnEnd {
                    autoSelectedNearPosition()
                }*/
        mPaint = getPorterDuffXferPaint()
        mMaskingPaint.isAntiAlias = true
        mMaskingPaint.color = Color.BLUE
        if (enableInitializeNotifySelectedItem) {
            selectPosition(0, false)
        }
    }


}