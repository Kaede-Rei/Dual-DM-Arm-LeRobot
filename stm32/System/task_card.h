#ifndef __TACK_CARD_H
#define __TASK_CARD_H

#include "stm32f10x.h"    // Device header

typedef struct{
    uint32_t start_time; //单位为ms
    uint32_t duration; //单位为ms
} TaskCard;


/******************** 基础函数 ********************/

/**
  *函    数：初始化任务卡片
  */
TaskCard* Task_Init(TaskCard* task,uint32_t start_time, uint32_t duration);

#endif
