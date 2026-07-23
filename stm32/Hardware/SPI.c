#include "SPI.h"                  // Device header

#ifdef USE_SPI1

/**
  *函    数：初始化SPI1
  */
void SPI1_Init(void){
    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);	//开启SPI1的时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA5和PA7引脚初始化为复用推挽输出
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA6引脚初始化为上拉输入
	
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;			//模式，选择为SPI主模式
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
	
	SPI_Cmd(SPI1, ENABLE);									//使能SPI1，开始运行
}

#endif

#ifdef USE_SPI2

/**
  *函    数：初始化SPI2
  */
void SPI2_Init(void){
    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//开启GPIOA的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);	//开启SPI1的时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将PA13和PA15引脚初始化为复用推挽输出
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将PA14引脚初始化为上拉输入
	
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;			//模式，选择为SPI主模式
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);
	
	SPI_Cmd(SPI2, ENABLE);									//使能SPI1，开始运行
}

#endif

#if defined(USE_SPI1) || defined(USE_SPI2)

/**
  * 函    数：SPI全双工交换数据
  * 参    数：SPIx        要使用的SPI外设（SPI1 或 SPI2）
  *           txBuf       发送数据缓冲区指针
  *           txLen       发送数据长度
  *           rxBuf       接收数据缓冲区指针
  *           rxLen       接收数据长度
  * 返 回 值：无
  * 补    充：使用轮询方式，全双工，SPI模式0；若不需要接收，rxBuf可传NULL
  */
void MySPI_Transfer(SPI_TypeDef* SPIx, uint8_t *txBuf, uint16_t txLen, uint8_t *rxBuf, uint16_t rxLen)
{
    uint16_t i;

    for(i = 0; i < txLen || i < rxLen; i++)
    {
        uint8_t txData = 0xFF; // 默认发送0xFF，如果发送缓冲区为空
        if(txBuf && i < txLen)
            txData = txBuf[i];

        // 等待发送数据寄存器空
        while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) != SET);

        SPI_I2S_SendData(SPIx, txData); // 发送数据

        // 等待接收数据寄存器非空
        while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) != SET);

        uint8_t rxData = SPI_I2S_ReceiveData(SPIx);

        if(rxBuf && i < rxLen)
            rxBuf[i] = rxData;
    }
}

#endif 
