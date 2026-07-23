#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f10x.h"
#include "hardware_config.h"

/******************** 系统滴答计数器 ********************/

extern volatile uint32_t SysTickCounter;

/******************** 系统滴答初始化 ********************/

/**
  * 函    数：SysTick初始化
  */
void SysTick_Init(uint32_t ticks);

/**
  * 函    数：获取系统滴答计数
  */
uint32_t SysTick_GetTick(void);

/**
  * 函    数：毫秒延时
  */
void Delay_ms(uint32_t ms);

/**
  * 函    数：计算任务持续时间
  */
uint32_t SysTick_Elapsed(uint32_t startTick);


/******************** SysTick中断处理 ********************/

/**
  * 函    数：SysTick中断函数
  */
void SysTick_Handler(void);

#endif

