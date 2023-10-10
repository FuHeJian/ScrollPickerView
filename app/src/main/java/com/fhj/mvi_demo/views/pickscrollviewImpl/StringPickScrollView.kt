package com.fhj.mvi_demo.views.pickscrollviewImpl

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Rect
import android.util.AttributeSet
import android.widget.Toast
import com.fhj.mvi_demo.views.PickScrollView

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
        orientation = ORIENTATION_VERTICAL
        showItemNum = 5
        paint.apply {
            isAntiAlias = true
            color = Color.WHITE
            textSize = 50f
        }
        data = arrayListOf("1", "2", "3", "4", "5", "6", "7", "8", "9")
        mListener = object : SelectListener<String> {
            override fun onSelect(position: Int, item: String) {
                Toast.makeText(context, item, Toast.LENGTH_SHORT).show()
            }
        }
    }

    companion object {

        val paint = Paint()

    }

    override fun getValueWidth(position: Int) = paint.measureText(data.get(position))

    val rectCache = Rect()
    override fun getValueHeight(position: Int):Float {
        var item = data.get(position)
        paint.getTextBounds(item, 0, item.length,rectCache)
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
                paint.color = Color.GREEN
            }

            -1, 1 -> {
                paint.color = Color.BLUE
            }

            else -> {
                paint.color = Color.LTGRAY
            }
        }
        paint.getTextBounds(item, 0, item.length, rect)
        canvas.drawText(item, 0f, rect.height().toFloat(), paint)
    }

}