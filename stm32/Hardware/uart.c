#include "uart.h"

RingBuff uart1TxRingBuff;
RingBuff uart2TxRingBuff;
RingBuff uart3TxRingBuff;

RingBuff uart1RxRingBuff;
RingBuff uart2RxRingBuff;
RingBuff uart3RxRingBuff;

uint32_t uart1RxLatestTime;
uint32_t uart2RxLatestTime;
uint32_t uart3RxLatestTime;

uint8_t UART3_Buffer[8];
uint8_t UART3_Index = 0;

#ifdef USE_UART1

/**
  *函    数：初始化UART1
  */
void UART1_Init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;   // 开漏输出，而非推挽
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA9引脚初始化为复用开漏输出
	
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA10引脚初始化为上拉输入
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = USE_UART1_BAUDRATE;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStructure);
    

	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
    /* 2 开启半双工 */
    USART_HalfDuplexCmd(USART1, ENABLE);
    
    USART_Cmd(USART1, ENABLE);
    
    UART_RingBuff_Init(&uart1TxRingBuff);
    UART_RingBuff_Init(&uart1RxRingBuff);

}

/**
  *函    数：UART1中断函数
  *补    充：需要根据通信协议完善中断处理函数
  */
void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {
        if(!UART_IsEmpty(&uart1TxRingBuff))
        {
            USART_SendData(USART1, UART_ReadRingBuff(&uart1TxRingBuff));
        }
        else
        {
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }

        USART_ClearITPendingBit(USART1, USART_IT_TXE);
    }
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){
        
        
        UART_WriteRingBuff(&uart2TxRingBuff,USART_ReceiveData(USART1));
        uart1RxLatestTime = SysTick_GetTick();
        
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
#endif


#ifdef USE_UART2

/**
  *函    数：初始化UART2
  */
void UART2_Init(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA2引脚初始化为复用推挽输出
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA3引脚初始化为上拉输入
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = USE_UART2_BAUDRATE;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &USART_InitStructure);
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART2, ENABLE);
    
    UART_RingBuff_Init(&uart2TxRingBuff);
    UART_RingBuff_Init(&uart2RxRingBuff);
}

/**
  *函    数：UART2中断函数
  *补    充：需要根据通信协议完善中断处理函数
  */
void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {
        if(!UART_IsEmpty(&uart2TxRingBuff))
        {
            USART_SendData(USART2, UART_ReadRingBuff(&uart2TxRingBuff));
        }
        else
        {
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
        }

        USART_ClearITPendingBit(USART2, USART_IT_TXE);
    }
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {

        UART_WriteRingBuff(&uart1TxRingBuff,USART_ReceiveData(USART2));
        uart2RxLatestTime = SysTick_GetTick();

        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

#endif

#ifdef USE_UART3

/**
  *函    数：初始化UART3
  */
void UART3_Init(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将PB10引脚初始化为复用推挽输出
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将PB11引脚初始化为上拉输入
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = USE_UART3_BAUDRATE;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART3, &USART_InitStructure);
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART3, ENABLE);
    
    UART_RingBuff_Init(&uart3TxRingBuff);
    UART_RingBuff_Init(&uart3RxRingBuff);
}

/**
  *函    数：UART3中断函数
  *补    充：需要根据通信协议完善中断处理函数
  */
void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
    {
        if(!UART_IsEmpty(&uart3TxRingBuff))
        {
            USART_SendData(USART3, UART_ReadRingBuff(&uart3TxRingBuff));
        }
        else
        {
            USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
        }

        USART_ClearITPendingBit(USART3, USART_IT_TXE);
    }
	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
	{
        UART_WriteRingBuff(&uart3RxRingBuff,USART_ReceiveData(USART3));
        uart3RxLatestTime = SysTick_GetTick();
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}
}

#endif

#if defined(USE_UART1) || defined(USE_UART2) || defined(USE_UART3)

/**
  * 函    数：串口发送一个字节
  * 参    数：Byte 要发送的一个字节
  * 参    数：USARTx 使用的串口
  * 返 回 值：无
  */
void Serial_SendByte(USART_TypeDef *USARTx, uint8_t Byte)
{
    USART_SendData(USARTx, Byte);
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
}

/**
  * 函    数：串口发送一个数组
  * 参    数：USARTx 使用的串口
  * 参    数：Array 要发送数组的首地址
  * 参    数：Length 要发送数组的长度
  * 返 回 值：无
  */
void Serial_SendArray(USART_TypeDef *USARTx, uint8_t *Array, uint16_t Length)
{
    uint16_t i;

    for(i = 0; i < Length; i++)
    {
        Serial_SendByte(USARTx, Array[i]);
    }
}

/**
  * 函    数：串口发送字符串
  * 参    数：USARTx 使用的串口
  * 参    数：String 要发送字符串的首地址
  * 返 回 值：无
  */
void Serial_SendString(USART_TypeDef *USARTx, char *String)
{
    while(*String)
    {
        Serial_SendByte(USARTx, *String++);
    }
}

#endif

/**
  * 函    数：初始化环形缓存结构体
  * 参    数：缓存结构体地址
  * 返 回 值：无
  */
void UART_RingBuff_Init(RingBuff* rb)
{
    rb->head = 0;
    rb->tail = 0;
}


/**
  * 函    数：读环形缓存
  * 参    数：缓存结构体地址
  * 返 回 值：读取的数据
  */
uint8_t UART_ReadRingBuff(RingBuff* rb)
{
    uint8_t data = rb->nums[rb->tail];
    rb->tail = (rb->tail + 1) % BUF_SIZE;
    return data;
}

/**
  * 函    数：写入环形缓存
  * 参    数：缓存结构体地址
  * 参    数：写入的数据
  * 返 回 值：是否为覆盖操作
  */
uint8_t UART_WriteRingBuff(RingBuff* rb, uint8_t data)
{
    uint16_t next = (rb->head + 1) % BUF_SIZE;

    if(next == rb->tail)
    {
        // 满了 → 覆盖最旧数据
        rb->tail = (rb->tail + 1) % BUF_SIZE;
    }

    rb->nums[rb->head] = data;
    rb->head = next;

    return 0;
}

uint8_t UART_IsEmpty(RingBuff* rb)
{
    return (rb->head == rb->tail);
}

uint16_t UART_GetBuffLength_Safe(RingBuff* rb)
{
    uint16_t head, tail;

    __disable_irq();
    head = rb->head;
    tail = rb->tail;
    __enable_irq();

    if(head >= tail)
        return head - tail;
    else
        return BUF_SIZE - (tail - head);
}

void Serial_StartSend(USART_TypeDef *USARTx){
    USARTx->CR1 |= USART_CR1_TXEIE;
}

void Serial_StopSend(USART_TypeDef *USARTx){
    USARTx->CR1 &= (~USART_CR1_TXEIE);
}

