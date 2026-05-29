#include "stm32f10x.h"                  // Device header
#include "systick.h"
#include "task_card.h"
#include "CAN.h"
#include "IIC.h"
#include "uart.h"
#include "oled.h"
#include "SysPWR.h"

#define SysLED_ON 1
#define SysLED_OFF 0

uint16_t SysLED_period = 25;
uint8_t SysLED_light=0;
uint8_t SysLED_drown=0;
int16_t SysLED_dir = 1;
uint8_t SysLED_state = SysLED_OFF;

TaskCard SysLED_Breath_Card;
TaskCard OLED_task;
TaskCard PWR_task;

void IWDG_Init(void);
void SysLED_Init(void);
void SysLED_State(uint8_t state);


int main(void)
{
    IWDG_Init();
    SysTick_Init(SystemCoreClock / 1000);
    SysLED_Init();
    UART1_Init();
    UART2_Init();
    UART3_Init();
    CAN_Config();
//    IIC1_Init();
//    SysPWR_INA219_Init();
//    OLED_Init();
//    OLED_ColorTurn(0);//0正常显示，1 反色显示
//    OLED_DisplayTurn(0);//0正常显示 1 屏幕翻转显示
    Task_Init(&OLED_task,0,300);
//    Task_Init(&PWR_task,0,100);
//    Task_Init(&SysLED_Breath_Card,0,SysLED_period);
	while (1)
	{
		IWDG_ReloadCounter();						//重装计数器，喂狗
        if(SysTick_Elapsed(SysLED_Breath_Card.start_time) >= SysLED_Breath_Card.duration){
            SysLED_Breath_Card.start_time = SysTick_GetTick();
            if(SysLED_state == SysLED_OFF){
                SysLED_State(SysLED_ON);
                SysLED_state = SysLED_ON;
                SysLED_light+=SysLED_dir;
                if(SysLED_light == SysLED_period) SysLED_dir = -SysLED_dir;
                SysLED_Breath_Card.duration = SysLED_light;
            }
            else if(SysLED_state == SysLED_ON){
                SysLED_State(SysLED_OFF);
                SysLED_state = SysLED_OFF;
                SysLED_drown = SysLED_period - SysLED_light;
                if(SysLED_drown == SysLED_period) SysLED_dir = -SysLED_dir;
                SysLED_Breath_Card.duration = SysLED_drown;
            }
        }
        /* 串口发送触发 */
        if(!UART_IsEmpty(&uart1TxRingBuff)) Serial_StartSend(USART1);
        if(!UART_IsEmpty(&uart2TxRingBuff)) Serial_StartSend(USART2);
        if(!UART_IsEmpty(&uart3TxRingBuff)) Serial_StartSend(USART3);

        /* 安全读取长度 */
        if(UART_GetBuffLength_Safe(&uart3RxRingBuff) >= 30)
        {
            UART3_CAN_Parse();
        }
        
//        if(UART_GetBuffLength(&uart3TxRingBuff) >= 16) Serial_StartSend(USART3);
//        if(!uart1TxRingBuff.isEmpty ){
//            while(!uart1TxRingBuff.isEmpty){
//                Serial_SendByte(USART1,UART_ReadRingBuff(&uart1TxRingBuff));
//            }
//        }
//        if(!uart2TxRingBuff.isEmpty ){
//            SysLED_State(SysLED_ON);
//            while(!uart2TxRingBuff.isEmpty){
//                Serial_SendByte(USART2,UART_ReadRingBuff(&uart2TxRingBuff));
//            }
//            SysLED_State(SysLED_OFF);
//        }
//        if(SysTick_Elapsed(PWR_task.start_time) >= PWR_task.duration){
//            PWR_task.start_time =  SysTick_GetTick();
//            SysPWR_INA219_Power_Value = SysPWR_INA219_Get_Power();
//            SysPWR_INA219_Current_Value = SysPWR_INA219_Get_Current();
//            SysPWR_INA219_Bus_Voltage = SysPWR_INA219_Get_Bus_Voltage();
//        }
//        if(SysTick_Elapsed(OLED_task.start_time) >= OLED_task.duration){
//            OLED_task.start_time =  SysTick_GetTick();
////            OLED_Clear();
//            OLED_ShowString(0,3,"A:",12,1);//6*12
//            OLED_ShowNum(12,3,SysPWR_INA219_Current_Value,5,12,1);
//            OLED_ShowString(42,3,"mA",12,1);//6*12
//            OLED_ShowString(68,3,"V:",12,1);//6*12
//            OLED_ShowNum(80,3,SysPWR_INA219_Bus_Voltage/1000,2,12,1);
//            OLED_ShowString(92,3,".",12,1);
//            OLED_ShowNum(98,3,SysPWR_INA219_Bus_Voltage % 1000,3,12,1);
//            OLED_ShowString(116,3,"V",12,1);//6*12
//            OLED_Refresh();
//        }
	}
}

void IWDG_Init(void){
	/*IWDG初始化*/
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);	//独立看门狗写使能
	IWDG_SetPrescaler(IWDG_Prescaler_16);			//设置预分频为16
	IWDG_SetReload(2499);							//设置重装值为2499，独立看门狗的超时时间为1000ms
	IWDG_ReloadCounter();							//重装计数器，喂狗
	IWDG_Enable();									//独立看门狗使能
}

void SysLED_Init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
//    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}

void SysLED_State(uint8_t state){
    if(state == SysLED_ON){
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    }
    else if(state == SysLED_OFF){
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
    }
}

