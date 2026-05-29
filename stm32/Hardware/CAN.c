#include "can.h"

RingBuff CANRxRingBuff;
uint32_t CANRxLatestTime;

void CAN_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    CAN_InitTypeDef CAN_InitStructure;
    CAN_FilterInitTypeDef CAN_FilterInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* CAN RX PA11 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* CAN TX PA12 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* CAN复位 */
    CAN_DeInit(CAN1);
    CAN_StructInit(&CAN_InitStructure);

    CAN_InitStructure.CAN_TTCM = DISABLE;
    CAN_InitStructure.CAN_ABOM = ENABLE;
    CAN_InitStructure.CAN_AWUM = DISABLE;
    CAN_InitStructure.CAN_NART = DISABLE;
    CAN_InitStructure.CAN_RFLM = DISABLE;
    CAN_InitStructure.CAN_TXFP = DISABLE;

    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;

    /* 1Mbps */
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    CAN_InitStructure.CAN_BS1 = CAN_BS1_8tq;
    CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;
    CAN_InitStructure.CAN_Prescaler = 3;

    CAN_Init(CAN1, &CAN_InitStructure);

    /* CAN过滤器 */
    CAN_FilterInitStructure.CAN_FilterNumber = 0;
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;

    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;

    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;

    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;

    CAN_FilterInit(&CAN_FilterInitStructure);

    /* CAN接收中断 */
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

    /* NVIC配置 */
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
    
    UART_RingBuff_Init(&CANRxRingBuff);
}

/* CAN发送 */
void CAN_SendFrame(uint16_t id,uint8_t *data,uint8_t len)
{
    CanTxMsg TxMessage;

    TxMessage.StdId = id;
    TxMessage.ExtId = 0;
    TxMessage.IDE = CAN_Id_Standard;
    TxMessage.RTR = CAN_RTR_Data;
    TxMessage.DLC = len;

    for(int i=0;i<len;i++)
        TxMessage.Data[i] = data[i];

    CAN_Transmit(CAN1,&TxMessage);
}

/* CAN接收处理 */
void CAN_RX_IRQHandler(void)
{
    CANRxLatestTime = SysTick_GetTick();
    CanRxMsg RxMessage;

    uint8_t frame[16];

    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

    frame[0] = 0xAA;          // header
    frame[1] = 0x11;          // CMD (可自定义)
    frame[2] = 0x00;          // reserved

    /* CAN ID (32bit little-endian) */
    frame[3] = RxMessage.StdId & 0xFF;
    frame[4] = (RxMessage.StdId >> 8) & 0xFF;
    frame[5] = 0x00;
    frame[6] = 0x00;

    /* DATA */
    for(uint8_t i = 0; i < 8; i++)
    {
        if(i < RxMessage.DLC)
            frame[7+i] = RxMessage.Data[i];
        else
            frame[7+i] = 0;
    }

    frame[15] = 0x55;         // tail

    /* 整帧写入发送缓冲 */
    for(uint8_t i=0;i<16;i++)
    {
        UART_WriteRingBuff(&uart3TxRingBuff,frame[i]);
    }

}


void UART3_CAN_Parse(void)
{
    static uint8_t buf[30];
    static uint8_t idx = 0;
    uint16_t id;
    uint8_t data[8];

    while(!UART_IsEmpty(&uart3RxRingBuff))
    {
        uint8_t ch = UART_ReadRingBuff(&uart3RxRingBuff);

        /* Align to LeRobot serial frame header: 0x55 0xAA */
        if(idx == 0)
        {
            if(ch != 0x55)
            {
                continue;
            }
            buf[idx++] = ch;
            continue;
        }

        if(idx == 1)
        {
            if(ch != 0xAA)
            {
                idx = 0;
                if(ch == 0x55)
                {
                    buf[idx++] = ch;
                }
                continue;
            }
            buf[idx++] = ch;
            continue;
        }

        buf[idx++] = ch;

        if(idx < 30)
        {
            continue;
        }

        /* Complete frame received */
        idx = 0;

        /* Optional sanity check: expected payload length in protocol template */
        if(buf[2] != 0x1E)
        {
            continue;
        }

        /* CAN ID in LeRobot frame is little-endian at byte 13/14 */
        id = ((uint16_t)buf[14] << 8) | buf[13];

        /* CAN data payload starts at byte 21 */
        for(int i=0;i<8;i++)
        {
            data[i] = buf[21+i];
        }

        CAN_SendFrame(id,data,8);
    }
}
