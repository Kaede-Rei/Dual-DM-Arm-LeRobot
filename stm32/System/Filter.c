#include "filter.h"


/**
 * 函    数：滑动平均滤波
 * 参    数：滤波器结构体，输入样本
 * 返 回 值：平均值
 */
int16_t MovingAvg_Calc(MovingAvgFilter *filter, int16_t sample)
{
    if(!filter->init)
    {
        for(int i=0;i<FILTER_SIZE;i++)
        {
            filter->buf[i] = sample;
        }

        filter->sum = sample * FILTER_SIZE;
        filter->init = 1;
    }

    filter->sum -= filter->buf[filter->index];

    filter->buf[filter->index] = sample;

    filter->sum += sample;

    filter->index++;
    if(filter->index >= FILTER_SIZE)
        filter->index = 0;

    return filter->sum / FILTER_SIZE;
}
