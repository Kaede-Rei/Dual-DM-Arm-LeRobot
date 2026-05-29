#ifndef __ADC1_H
#define __ADC1_H

#include "stm32f10x.h"
#include "hardware_config.h"


#ifdef USE_ADC1


/******************** ADC初始化函数 ********************/
/**
  * 函    数：初始化ADC1指定通道
  */
void ADC1_Init_Channel(uint8_t channel);

/******************** ADC读取函数 ********************/
/**
  * 函    数：读取ADC1指定通道的数值
  */
uint16_t ADC1_Read(uint8_t channel);

#endif


#endif
