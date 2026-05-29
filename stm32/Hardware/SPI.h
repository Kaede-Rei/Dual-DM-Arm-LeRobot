#ifndef __SPI_H
#define __SPI_H

#include "stm32f10x.h"
#include "hardware_config.h"

/******************** SPI 基础函数 ********************/

#if defined(USE_SPI1) || defined(USE_SPI2)

/**
  * 函    数：SPI全双工交换数据
  */
void MySPI_Transfer(SPI_TypeDef* SPIx, uint8_t *txBuf, uint16_t txLen, uint8_t *rxBuf, uint16_t rxLen);

#endif


/******************** SPI 初始化 ********************/

#ifdef USE_SPI1
void SPI1_Init(void);
#endif

#ifdef USE_SPI2
void SPI2_Init(void);
#endif


#endif
