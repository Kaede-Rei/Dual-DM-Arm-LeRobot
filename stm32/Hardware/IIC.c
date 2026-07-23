#include "IIC.h"


#ifdef USE_IIC1

/**
  *函    数：初始化IIC1
  */
void IIC1_Init(void){
    
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	I2C_InitTypeDef I2C_InitStructure;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_ClockSpeed = 50000;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_Init(I2C1, &I2C_InitStructure);
	
	I2C_Cmd(I2C1, ENABLE);
}


#endif

#ifdef USE_IIC2

/**
  *函    数：初始化IIC2
  */
void IIC2_Init(void){
    
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	I2C_InitTypeDef I2C_InitStructure;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_ClockSpeed = 50000;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_Init(I2C2, &I2C_InitStructure);
	
	I2C_Cmd(I2C2, ENABLE);
}


#endif


#if defined(USE_IIC1) || defined(USE_IIC2)

/**
  * 函    数：IIC等待事件
  * 参    数：同I2C_CheckEvent
  * 返 回 值：无
  */
void I2C_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)
{
	uint32_t Timeout;
	Timeout = 10000;									//给定超时计数时间
	while (I2C_CheckEvent(I2Cx, I2C_EVENT) != SUCCESS)	//循环等待指定事件
	{
		Timeout --;										//等待时，计数值自减
		if (Timeout == 0)								//自减到0后，等待超时
		{
			/*超时的错误处理代码，可以添加到此处*/
			break;										//跳出等待，不等了
		}
	}
}

/**
  * 函    数：IIC写寄存器
  * 参    数：I2Cx 使用的I2C
  * 参    数：RegAddress 寄存器地址
  * 参    数：Data 要写入的数据
  * 参    数：Length 数据长度
  * 返 回 值：无
  */
void IIC_WriteReg(I2C_TypeDef* I2Cx, uint8_t DevAddr, uint8_t RegAddress, uint8_t* Data, uint8_t Length)
{
    uint8_t i;

    I2C_GenerateSTART(I2Cx, ENABLE);
    I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT);

    I2C_Send7bitAddress(I2Cx, DevAddr, I2C_Direction_Transmitter);
    I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);

    I2C_SendData(I2Cx, RegAddress);
    I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED);

    for(i = 0; i < Length; i++)
    {
        I2C_SendData(I2Cx, Data[i]);
        I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED);
    }

    I2C_GenerateSTOP(I2Cx, ENABLE);
}

/**
  * 函    数：IIC读寄存器
  * 参    数：I2Cx 使用的I2C
  * 参    数：DevAddr 设备地址
  * 参    数：RegAddress 寄存器地址
  * 参    数：Data 数据存放地址
  * 参    数：Length 读取长度
  * 返 回 值：无
  */
void IIC_ReadReg(I2C_TypeDef* I2Cx, uint8_t DevAddr, uint8_t RegAddress, uint8_t* Data, uint8_t Length)
{
    uint8_t i;

    I2C_GenerateSTART(I2Cx, ENABLE);
    I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT);

    I2C_Send7bitAddress(I2Cx, DevAddr, I2C_Direction_Transmitter);
    I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);

    I2C_SendData(I2Cx, RegAddress);
    I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED);

    I2C_GenerateSTART(I2Cx, ENABLE);
    I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT);

    I2C_Send7bitAddress(I2Cx, DevAddr, I2C_Direction_Receiver);
    I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);

    for(i = 0; i < Length; i++)
    {
        if(i == Length - 1)
        {
            I2C_AcknowledgeConfig(I2Cx, DISABLE);
            I2C_GenerateSTOP(I2Cx, ENABLE);
        }

        I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED);
        Data[i] = I2C_ReceiveData(I2Cx);
    }

    I2C_AcknowledgeConfig(I2Cx, ENABLE);
}


#endif

