package com.ext.extview.pickscrollviewImpl

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Rect
import android.util.AttributeSet
import android.util.Log
import android.widget.Toast
import com.ext.extview.PickScrollView
import kotlin.math.absoluteValue

/**
@author: fuhejian
@date: 2023/10/8
 */
class StringPickScrollView : PickScrollView<String> {

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
        orientation = ORIENTATION_HORIZONTAL
        overScroll = 30f
        showItemNum = 6f
        paint.apply {
            isAntiAlias = true
            color = Color.WHITE
            textSize = 50f
        }
        data = arrayListOf("1", "2", "3", "4", "5", "6", "7", "8", "9")
        mListener = object : SelectListener<String> {
            override fun onSelect(pickview: PickScrollView<String>, position: Int, item: String) {
                Toast.makeText(context, item, Toast.LENGTH_SHORT).show()
            }
        }

    }

    companion object {

        val paint = Paint()

    }

    override fun getValueWidth(position: Int) = paint.measureText(data.get(position))

    val rectCache = Rect()
    override fun getValueHeight(position: Int): Float {
        var item = data.get(position)
        paint.getTextBounds(item, 0, item.length, rectCache)
        return rectCache.height().toFloat()
    }

    override fun drawItem(
        canvas: Canvas,
        item: String,
        offset: Float,
        position: Int,
        nearSelectedIndex: Int
    ) {
        val rect = rectCache
        when (nearSelectedIndex) {
            0 -> {
                paint.color = Color.WHITE
            }

            -1, 1 -> {
                paint.color = Color.GRAY
            }

            else -> {
                paint.color = Color.LTGRAY
            }
        }
        if (nearSelectedIndex.absoluteValue > 1) {
            paint!!.color = Color.WHITE
        } else {
            paint!!.color = computeGradientColor(Color.GREEN, Color.WHITE, offset.absoluteValue)
        }
        paint.getTextBounds(item, 0, item.length, rect)
        canvas.drawText(item, 0f, height / 2f + rect.height().toFloat() / 2f, paint)


        if (nearSelectedIndex == 1) {
            Log.d(
                "日志",
                "绘制位置: " + position + "相对距离：" + offset.absoluteValue + ",nearSelectedIndex:" + nearSelectedIndex
            )
        }

    }

    fun computeGradientColor(startColor: Int, endColor: Int, rate: Float): Int {
        var rate = rate
        if (rate < 0) {
            rate = 0f
        }
        if (rate > 1) {
            rate = 1f
        }
        val alpha = Color.alpha(endColor) - Color.alpha(startColor)
        val red = Color.red(endColor) - Color.red(startColor)
        val green = Color.green(endColor) - Color.green(startColor)
        val blue = Color.blue(endColor) - Color.blue(startColor)
        return Color.argb(
            Math.round(Color.alpha(startColor) + alpha * rate),
            Math.round(Color.red(startColor) + red * rate),
            Math.round(Color.green(startColor) + green * rate),
            Math.round(Color.blue(startColor) + blue * rate)
        )
    }

}