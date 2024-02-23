package com.fhj.mvi_demo

import android.content.Context
import android.graphics.Rect
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View

/**
@author: fuhejian
@date: 2023/11/9
 */
class SucView:View {
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

//    var sucEdge = SuctionEdge()
    init {
//        sucEdge.suctionView = this
    }

    var cacheR = Rect()

    override fun onSizeChanged(w: Int, h: Int, oldw: Int, oldh: Int) {
        super.onSizeChanged(w, h, oldw, oldh)
        val p = parent as View

        cacheR.set(p.paddingLeft,p.paddingTop,p.width - p.paddingRight,p.bottom- p.paddingBottom)
//        sucEdge.rect = cacheR
    }

    override fun onTouchEvent(event: MotionEvent?): Boolean {
//        return sucEdge.process(event!!)
        return true
    }
}