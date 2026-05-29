#include "stm32f10x.h"                  // Device header
#include "IIC.h"
#include "INA219_Reg.h"
#include "Filter.h"

#define INA219_ADDRESS		0x80		//INA219的I2C从机地址

MovingAvgFilter current_filter = {0};
MovingAvgFilter voltage_filter = {0};
MovingAvgFilter power_filter   = {0};

/**
  * 函    数：MPU6050等待事件
  * 参    数：同I2C_CheckEvent
  * 返 回 值：无
  */
void SysPWR_INA219_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)
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
  * 函    数：INA219写寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考INA219手册的寄存器描述
  * 参    数：Data 要写入寄存器的数据，范围：0x0000~0xFFFF
  * 返 回 值：无
  */
void SysPWR_INA219_WriteReg(uint8_t RegAddress, uint16_t Data)
{
	I2C_GenerateSTART(I2C1, ENABLE);										//硬件I2C生成起始条件
	SysPWR_INA219_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C1, INA219_ADDRESS, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
	SysPWR_INA219_WaitEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
	I2C_SendData(I2C1, RegAddress);											//硬件I2C发送寄存器地址
	SysPWR_INA219_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING);			//等待EV8
	
	I2C_SendData(I2C1, Data >> 8);												//硬件I2C发送数据
	SysPWR_INA219_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED);				//等待EV8_2
    
	I2C_SendData(I2C1, Data & 0xFF);												//硬件I2C发送数据
	SysPWR_INA219_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED);				//等待EV8_2
	
	I2C_GenerateSTOP(I2C1, ENABLE);											//硬件I2C生成终止条件
}

/**
  * 函    数：INA219读寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考INA219手册的寄存器描述
  * 返 回 值：读取寄存器的数据，范围：0x0000~0xFFFF
  */
uint16_t SysPWR_INA219_ReadReg(uint8_t RegAddress)
{
    uint8_t MSByte, LSByte;
    uint16_t Data;

    I2C_GenerateSTART(I2C1, ENABLE);                                    
    SysPWR_INA219_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT);               // EV5
    
    I2C_Send7bitAddress(I2C1, INA219_ADDRESS, I2C_Direction_Transmitter);
    SysPWR_INA219_WaitEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED); // EV6
    
    I2C_SendData(I2C1, RegAddress);                                     
    SysPWR_INA219_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED);          // EV8_2
    

    /* 重新开始读取 */
    I2C_GenerateSTART(I2C1, ENABLE);
    SysPWR_INA219_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT);               // EV5
    
    I2C_Send7bitAddress(I2C1, INA219_ADDRESS, I2C_Direction_Receiver);
    SysPWR_INA219_WaitEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);    // EV6
    

    /* 读取第1个字节 */
    SysPWR_INA219_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED);             // EV7
    MSByte = I2C_ReceiveData(I2C1);


    /* 读取最后1个字节 */
    I2C_AcknowledgeConfig(I2C1, DISABLE);   // 关闭ACK
    I2C_GenerateSTOP(I2C1, ENABLE);         // 发送STOP

    SysPWR_INA219_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED);
    LSByte = I2C_ReceiveData(I2C1);


    I2C_AcknowledgeConfig(I2C1, ENABLE);    // 恢复ACK


    Data = (MSByte << 8) | LSByte;

    return Data;
}

/**
  *函    数：INA219初始化
  *参    数：无
  *返 回 值：无
*/
void SysPWR_INA219_Init(void){
    IIC1_Init();
    SysPWR_INA219_WriteReg(CALIBRATION,0x1000);
}

/**
  *函    数：获取总线电流值
  *参    数：无
  *返 回 值：总线电流值，单位mA
*/
int16_t SysPWR_INA219_Get_Current(void)
{
    int16_t raw = (int16_t)SysPWR_INA219_ReadReg(CURRENT);

    return MovingAvg_Calc(&current_filter, raw);
}

/**
  *函    数：获取负载功率
  *参    数：无
  *返 回 值：负载功率，单位mW
*/
int16_t SysPWR_INA219_Get_Power(void)
{
    int16_t raw = (int16_t)SysPWR_INA219_ReadReg(POWER) * 20;

    return MovingAvg_Calc(&power_filter, raw);
}

/**
  *函    数：获取总线电压
  *参    数：无
  *返 回 值：总线电压，单位mV
*/
uint16_t SysPWR_INA219_Get_Bus_Voltage(void)
{
    uint16_t reg = SysPWR_INA219_ReadReg(BUS_VOLTAGE);

    if((reg & 0x0003) == 0x0002)   // CNVR=1 OVF=0
    {
        int16_t voltage = (reg >> 3) * 4;  // mV

        return MovingAvg_Calc(&voltage_filter, voltage);
    }

    return 0xFFFF;
}
