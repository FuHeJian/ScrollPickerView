package com.fhj.mvi_demo.views

import android.content.Context
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View

/**
@author: fuhejian
@date: 2024/3/29
 */
class TestView(context: Context?, attrs: AttributeSet?) : View(context, attrs) {

    override fun onLayout(changed: Boolean, left: Int, top: Int, right: Int, bottom: Int) {
        super.onLayout(changed, left, top, right, bottom)
        setOnClickListener(object : OnClickListener {
            override fun onClick(v: View?) {
                println("被点击了")
            }
        });
        setOnClickListener(object : OnClickListener {
            override fun onClick(v: View?) {
                println("被点击了")
            }
        });
        setOnLongClickListener(object : OnLongClickListener {
            override fun onLongClick(v: View?): Boolean {
                println("被长按了");
                return true;
            }
        });
    }

    override fun onTouchEvent(event: MotionEvent?): Boolean {
        println(this.id.toString() + event)
        return super.onTouchEvent(event)
    }

}