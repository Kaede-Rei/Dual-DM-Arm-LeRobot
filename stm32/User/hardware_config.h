#ifndef __HARDWARE_CONFIG_H
#define __HARDWARE_CONFIG_H

/************** UART **************/

#define USE_UART1
#define USE_UART1_BAUDRATE 115200 //小臂信号发送串口     *注意改为半双工

#define USE_UART2
#define USE_UART2_BAUDRATE 115200 //小臂USB对应串口

#define USE_UART3
#define USE_UART3_BAUDRATE 115200 //大臂USB对应串口

/************** 根据需要完善对应串口中断函数 **************/


/************** IIC **************/

#define USE_IIC1
// #define USE_IIC2

/************** IIC **************/


/************** SPI **************/

//#define USE_SPI1
// #define USE_SPI2

/************** 根据需要使用IO模拟NSS **************/


/************** ADC1 **************/

//#define USE_ADC1

/************** ADC1 **************/

#endif




