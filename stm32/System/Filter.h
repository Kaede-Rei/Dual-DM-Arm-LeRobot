#ifndef __FILTER_H
#define __FILTER_H

#include "stm32f10x.h"

#define FILTER_SIZE 20

typedef struct
{
    int32_t sum;
    int16_t buf[FILTER_SIZE];
    uint8_t index;
    uint8_t init;
} MovingAvgFilter;

int16_t MovingAvg_Calc(MovingAvgFilter *filter, int16_t sample);

#endif


