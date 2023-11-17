package com.ext.extview

import android.content.Context
import android.graphics.Canvas
import android.util.AttributeSet
import android.view.View
import java.util.concurrent.CopyOnWriteArrayList

/**
@author: fuhejian
@date: 2023/10/9
 */
open class SimpleView :View {
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

    override fun onDrawForeground(canvas: Canvas?) {
        super.onDrawForeground(canvas)
        mOnDrawLater.forEach {
            it.onDrawLater()
        }
    }

    private var mOnDrawLater:CopyOnWriteArrayList<OnDrawLater> = CopyOnWriteArrayList()

    fun addOnDrawLaterListener(listener: OnDrawLater){
        mOnDrawLater.add(listener)
    }
    fun removeOnDrawLaterListener(listener: OnDrawLater) = mOnDrawLater.removeIf({
        it.equals(listener)
    })

    interface OnDrawLater{
        fun onDrawLater();
    }

}

inline fun SimpleView.doOnDrawLater(crossinline listener: ()->Unit){
    this.addOnDrawLaterListener(object : SimpleView.OnDrawLater {
        override fun onDrawLater() {
            listener()
            removeOnDrawLaterListener(this)
        }
    })
}