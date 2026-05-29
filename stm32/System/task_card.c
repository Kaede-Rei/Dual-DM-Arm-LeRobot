#include "task_card.h"

/**
  * 函    数：初始化任务卡片
  * 参    数：*task 任务卡片指针
  * 参    数：start_time 任务开始的时间
  * 参    数：duration 任务持续的时间
  * 返 回 值：无
  */
TaskCard* Task_Init(TaskCard* task,uint32_t start_time, uint32_t duration){
    task->start_time = start_time;
    task->duration = duration;
    return task;
}
