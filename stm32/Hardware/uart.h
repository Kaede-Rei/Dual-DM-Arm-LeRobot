#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"
#include "hardware_config.h"
#include "CAN.h"
#include "systick.h"

#define BUF_SIZE 1024

typedef struct
{
    volatile uint16_t head;
    volatile uint16_t tail;
    uint8_t nums[BUF_SIZE];
}RingBuff;

extern RingBuff uart1TxRingBuff;
extern RingBuff uart2TxRingBuff;
extern RingBuff uart3TxRingBuff;

extern RingBuff uart1RxRingBuff;
extern RingBuff uart2RxRingBuff;
extern RingBuff uart3RxRingBuff;

extern RingBuff CANRxRingBuff;

extern uint32_t CANRxLatestTime;

extern uint32_t uart1RxLatestTime;
extern uint32_t uart2RxLatestTime;
extern uint32_t uart3RxLatestTime;

void UART_RingBuff_Init(RingBuff* Ringbuff);
uint8_t UART_ReadRingBuff(RingBuff* Ringbuff);
uint8_t UART_WriteRingBuff(RingBuff* Ringbuff, uint8_t data);
void Serial_StartSend(USART_TypeDef *USARTx);
void Serial_StopSend(USART_TypeDef *USARTx);
uint16_t UART_GetBuffLength_Safe(RingBuff* rb);
uint8_t UART_IsEmpty(RingBuff* rb);

#if defined(USE_UART1) || defined(USE_UART2) || defined(USE_UART3)
/******************** 串口发送函数 ********************/

/**
  * 函    数：发送一个字节
  */
void Serial_SendByte(USART_TypeDef *USARTx, uint8_t Byte);

/**
  * 函    数：发送数组
  */
void Serial_SendArray(USART_TypeDef *USARTx, uint8_t *Array, uint16_t Length);

/**
  * 函    数：发送字符串
  */
void Serial_SendString(USART_TypeDef *USARTx, char *String);

/**
  * 函    数：发送数字
  */
void Serial_SendNumber(USART_TypeDef *USARTx, uint32_t Number, uint8_t Length);

#endif

/******************** 串口初始化 ********************/

#ifdef USE_UART1
void UART1_Init(void);
#endif

#ifdef USE_UART2
void UART2_Init(void);
#endif

#ifdef USE_UART3
void UART3_Init(void);
#endif

#endif
