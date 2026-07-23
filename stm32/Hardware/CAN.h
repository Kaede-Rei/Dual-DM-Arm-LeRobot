#ifndef __CAN_H
#define __CAN_H

#include "stm32f10x.h"
#include "systick.h"
#include "uart.h"


#define FRAME_HEAD 0xAA



/* º¯ÊýÉùÃ÷ */

void CAN_Config(void);

void CAN_SendData(uint8_t *data, uint8_t len);

void CAN_RX_IRQHandler(void);

void UART3_CAN_Parse(void);

#endif



