#include "systick.h"

volatile uint32_t SysTickCounter = 0;


/**
  * 函    数：SysTick初始化
  * 参    数：ticks - 每多少个时钟节拍触发一次SysTick中断
  * 返 回 值：无
  * 补    充：通常用于系统时间管理和任务时间片调度
  */
void SysTick_Init(uint32_t ticks)
{
    // 配置SysTick
    // SystemCoreClock为系统时钟频率
    // ticks = SystemCoreClock / 1000 表示1ms中断一次
    if (SysTick_Config(ticks))
    {
        // 配置失败，进入死循环
        while(1);
    }
}

/**
  * 函    数：获取系统滴答计数
  * 参    数：无
  * 返 回 值：当前系统滴答数（毫秒）
  * 补    充：用于任务延时或时间统计
  */
uint32_t SysTick_GetTick(void)
{
    return SysTickCounter;
}

/**
  * 函    数：毫秒延时
  * 参    数：ms - 延时时间（毫秒）
  * 返 回 值：无
  * 补    充：阻塞式延时，适合短时间延时
  */
void Delay_ms(uint32_t ms)
{
    uint32_t start = SysTick_GetTick();
    while ((SysTick_GetTick() - start) < ms);
}


/**
  * 函    数：计算从过去某时刻到当前的时间差
  * 参    数：startTick - 过去的系统滴答
  * 返 回 值：时间差（毫秒）
  * 补    充：自动处理SysTickCounter溢出情况
  */
uint32_t SysTick_Elapsed(uint32_t startTick)
{
    uint32_t now = SysTick_GetTick();

    if(now >= startTick)
        return now - startTick;
    else
        return (0xFFFFFFFF - startTick + 1) + now; // 处理溢出
}

/**
  * 函    数：SysTick中断函数
  * 参    数：无
  * 返 回 值：无
  * 补    充：在中断中增加系统滴答计数，可用于任务时间片调度
  */

