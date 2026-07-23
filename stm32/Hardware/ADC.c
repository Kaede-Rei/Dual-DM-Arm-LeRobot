#include "ADC.h"

#ifdef USE_ADC1

/**
  * 函    数：初始化ADC1指定通道
  * 参    数：channel - ADC通道号（0~9）
  * 返 回 值：无
  * 补    充：自动配置对应GPIO口并进行ADC1初始化与校准
  */
void ADC1_Init_Channel(uint8_t channel)
{
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;

    // 打开ADC和GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    if(channel <= 7) // PA0~PA7 对应 IN0~IN7
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        GPIOx = GPIOA;
        GPIO_Pin = (1 << channel); // GPIO_Pin0=0x0001, Pin1=0x0002 ...
    }
    else if(channel == 8) // PB0 对应 IN8
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        GPIOx = GPIOB;
        GPIO_Pin = GPIO_Pin_0;
    }
    else if(channel == 9) // PB1 对应 IN9
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        GPIOx = GPIOB;
        GPIO_Pin = GPIO_Pin_1;
    }
    else
    {
        return; // 无效通道，直接返回
    }

    // 配置GPIO为模拟输入
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOx, &GPIO_InitStructure);

    // 配置ADC1
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Cmd(ADC1, ENABLE);

    // ADC校准
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1) == SET);
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) == SET);
}

/**
  * 函    数：读取ADC1指定通道的数值
  * 参    数：channel - ADC通道号（0~9）
  * 返 回 值：ADC转换结果（0~4095）
  * 补    充：单次转换模式，返回右对齐数据
  */
uint16_t ADC1_Read(uint8_t channel)
{
    // 设置要转换的通道和采样时间
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_55Cycles5);

    // 启动转换
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    // 等待转换完成
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

    // 读取数据
    return ADC_GetConversionValue(ADC1);
}

#endif
