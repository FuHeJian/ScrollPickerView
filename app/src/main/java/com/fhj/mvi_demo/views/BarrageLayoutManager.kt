package com.fhj.mvi_demo.views

import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.RecyclerView.LayoutManager

/**
@author: fuhejian
@date: 2023/10/12
 */
class BarrageLayoutManager:LayoutManager {

    constructor()

    override fun generateDefaultLayoutParams(): RecyclerView.LayoutParams {
        return  RecyclerView.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,ViewGroup.LayoutParams.WRAP_CONTENT)
    }




}