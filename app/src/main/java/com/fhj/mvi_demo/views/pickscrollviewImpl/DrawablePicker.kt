package com.fhj.mvi_demo.views.pickscrollviewImpl

import android.content.Context
import android.graphics.Canvas
import android.graphics.Rect
import android.graphics.drawable.Drawable
import android.util.AttributeSet
import android.widget.Toast
import com.fhj.mvi_demo.R
import com.fhj.mvi_demo.views.PickScrollView

/**
@author: fuhejian
@date: 2023/10/10
 */

class DrawablePicker : PickScrollView<Drawable> {

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

        data = arrayListOf(
            context.getDrawable(R.drawable.wifi_rssi_0),
            context.getDrawable(R.drawable.wifi_rssi_1),
            context.getDrawable(R.drawable.wifi_rssi_2),
            context.getDrawable(R.drawable.wifi_rssi_3)
        )

        mListener = object : SelectListener<Drawable> {
            override fun onSelect(position: Int, item: Drawable) {
                Toast.makeText(context, position.toString(), Toast.LENGTH_SHORT).show()
            }
        }

    }

    override fun getValueWidth(position: Int): Float {
        if (data.get(position).bounds.height() != height) {
            val ratio =
                data.get(position).intrinsicWidth.toFloat() / data.get(position).intrinsicHeight.toFloat()
            data.get(position).bounds = Rect(0, 0, (height * ratio).toInt(), height)
        }
        return data.get(position).intrinsicWidth.toFloat()
    }

    override fun getValueHeight(position: Int) = 0f

    override fun drawItem(
        canvas: Canvas,
        item: Drawable,
        offset: Float,
        position: Int,
        offsetSelectedIndex: Int
    ) {
        item.draw(canvas)
    }

}