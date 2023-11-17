package com.ext.extview.utils

import android.content.Context
import android.graphics.Rect

/**
@author: fuhejian
@date: 2023/10/23
 */

fun Int.dp(context:Context) = context.resources.displayMetrics.density * this

val rectCache = Rect();
operator fun Rect.minus(r:Rect):Rect{
    rectCache.set(this.left - r.left,this.top - r.top,this.right - r.right,this.bottom - r.bottom)
    return rectCache
}

infix fun Rect.minusCenter(r:Rect):Rect{
    rectCache.set(this.left - r.centerX(),this.top - r.centerY(),this.right - r.centerX(),this.bottom - r.centerY())
    return rectCache
}