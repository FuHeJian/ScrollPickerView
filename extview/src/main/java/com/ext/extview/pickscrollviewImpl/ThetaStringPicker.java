package com.ext.extview.pickscrollviewImpl;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.ext.extview.PickScrollView;
import com.ext.extview.utils.ColorUtil;
import com.ext.extview.utils.Colors;
import com.ext.extview.utils.SizeUtilKt;

import java.util.ArrayList;

/**
 * @author: fuhejian
 * @date: 2023/10/20
 */
public class ThetaStringPicker extends PickScrollView<String> {


    public ThetaStringPicker(Context context) {
        this(context, null);
    }

    public ThetaStringPicker(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public ThetaStringPicker(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        this(context, attrs, defStyleAttr, 0);
    }

    public ThetaStringPicker(Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        init();
    }

    private Paint textPaint = new Paint();

    private float textSize = SizeUtilKt.dp(16, getContext());

    public void init() {
        textPaint.setAntiAlias(true);
        textPaint.setTextSize(textSize);
        ArrayList<String> objects = new ArrayList<>();
        objects.add("none");
/*        for (int i = 0; i < 10; i++) {
            objects.add(String.valueOf(i));
        }*/
        setData(objects);
        this.setShowItemNum(8);
    }

    private Rect cacheRect = new Rect();

    @Override
    public void drawItem(@NonNull Canvas canvas, String s, float offset, int position, int index) {
        int color = Colors.WHITE;
        int absIndex = Math.abs(index);
        float offsetIndex = Math.abs(offset);
        if (absIndex >= 4) {
            color = Colors.WHITE_15;
        }
        if (absIndex == 0) {
            Log.d("日志", "offset: " + offset + ",index" + index);
        }
        if (offset < 0) {//从左到右
            if (absIndex == 0) {
                color = ColorUtil.computeGradientColor(Colors.WHITE, Colors.WHITE_60, offsetIndex);
            } else if (absIndex == 1) {
                if (index == -1) {
                    color = ColorUtil.computeGradientColor(Colors.WHITE_60, Colors.WHITE, offsetIndex);
                } else {
                    color = ColorUtil.computeGradientColor(Colors.WHITE_60, Colors.WHITE_30, offsetIndex);
                }
            } else if (absIndex == 2) {
                if (index == -2) {
                    color = ColorUtil.computeGradientColor(Colors.WHITE_30, Colors.WHITE_60, offsetIndex);
                } else {
                    color = ColorUtil.computeGradientColor(Colors.WHITE_30, Colors.WHITE_15, offsetIndex);
                }
            } else if (absIndex == 3) {
                if (index == -3) {
                    color = ColorUtil.computeGradientColor(Colors.WHITE_15, Colors.WHITE_30, offsetIndex);
                } else {
                    color = Colors.WHITE_15;
                }
            }
        } else if (offset > 0) {//从右到左
            if (absIndex == 0) {
                color = ColorUtil.computeGradientColor(Colors.WHITE, Colors.WHITE_60, offsetIndex);
            } else if (absIndex == 1) {
                if (index == 1) {
                    color = ColorUtil.computeGradientColor(Colors.WHITE_60, Colors.WHITE, offsetIndex);
                } else {
                    color = ColorUtil.computeGradientColor(Colors.WHITE_60, Colors.WHITE_30, offsetIndex);
                }
            } else if (absIndex == 2) {
                if (index == 2) {
                    color = ColorUtil.computeGradientColor(Colors.WHITE_30, Colors.WHITE_60, offsetIndex);
                } else {
                    color = ColorUtil.computeGradientColor(Colors.WHITE_30, Colors.WHITE_15, offsetIndex);
                }
            } else if (absIndex == 3) {
                if (index == 3) {
                    color = ColorUtil.computeGradientColor(Colors.WHITE_15, Colors.WHITE_30, offsetIndex);
                } else {
                    color = Colors.WHITE_15;
                }
            }
        } else {
            if (absIndex == 0) {
                color = Colors.WHITE;
            } else if (absIndex == 1) {
                color = Colors.WHITE_60;
            } else if (absIndex == 2) {
                color = Colors.WHITE_30;
            } else if (absIndex == 3) {
                color = Colors.WHITE_15;
            }
        }

        textPaint.setColor(color);
        canvas.drawText(s, 0f, getHeight() / 2f + cacheRect.height() / 2f, textPaint);

    }


    @Override
    public float getValueHeight(int i) {
        String s = getSelectedItem(i);
        textPaint.getTextBounds(s, 0, s.length(), cacheRect);
        return cacheRect.width();
    }

    @Override
    public float getValueWidth(int i) {
        return 0;
    }

}
