package com.fhj.mvi_demo.views

import android.content.Context
import android.util.AttributeSet

/**
@author: fuhejian
@date: 2023/10/12
 */
class Barrage : com.ext.extview.SimpleView {

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



    }

}