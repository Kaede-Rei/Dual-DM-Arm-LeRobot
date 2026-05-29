#ifndef __SYSPWR_H
#define __SYSPWR_H

int16_t SysPWR_INA219_Power_Value = 0;
int16_t SysPWR_INA219_Current_Value = 0;
uint16_t SysPWR_INA219_Bus_Voltage = 0;


void SysPWR_INA219_WriteReg(uint8_t RegAddress, uint16_t Data);
uint16_t SysPWR_INA219_ReadReg(uint8_t RegAddress);
void SysPWR_INA219_Init(void);
uint16_t SysPWR_INA219_Get_Current(void);
uint16_t SysPWR_INA219_Get_Power(void);
uint16_t SysPWR_INA219_Get_Bus_Voltage(void);

#endif
