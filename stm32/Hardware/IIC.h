#ifndef __IIC_H
#define __IIC_H

#include "stm32f10x.h"
#include "hardware_config.h"

/******************** IIC 基础函数 ********************/

#if defined(USE_IIC1) || defined(USE_IIC2)

/**
  * 函    数：IIC等待事件
  */
void I2C_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT);

/**
  * 函    数：IIC写寄存器
  */
void IIC_WriteReg(I2C_TypeDef* I2Cx,
                  uint8_t DevAddr,
                  uint8_t RegAddress,
                  uint8_t* Data,
                  uint8_t Length);

/**
  * 函    数：IIC读寄存器
  */
void IIC_ReadReg(I2C_TypeDef* I2Cx,
                 uint8_t DevAddr,
                 uint8_t RegAddress,
                 uint8_t* Data,
                 uint8_t Length);

#endif


/******************** IIC 初始化 ********************/

#ifdef USE_IIC1
void IIC1_Init(void);
#endif

#ifdef USE_IIC2
void IIC2_Init(void);
#endif


#endif
