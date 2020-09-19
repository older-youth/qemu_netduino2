#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_syscfg.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_misc.h"
#include "gpio.h"

uint8_t BATStatus = 0; //锂电池充电状态状态,0位1是正在充电,1位1是故障
uint8_t PowerKeyStatus = 0; //电源状态状态,1是按下关机,下板不处理关机逻辑,由上板来处理. 如果只有下板不存在开关机插电源就开机

/**
  * @brief  IO口初始化
  * @param  no
  * @retval no
  */
void GpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    //串口USART
    //USART1
    GPIO_PinAFConfig(USART1_TX_GPIO_PORT, USART1_TX_GPIO_PIN, GPIO_AF_1);
    GPIO_InitStructure.GPIO_Pin = USART1_TX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(USART1_TX_GPIO_PORT, &GPIO_InitStructure);

    GPIO_PinAFConfig(USART1_RX_GPIO_PORT, USART1_RX_GPIO_PIN, GPIO_AF_1);
    GPIO_InitStructure.GPIO_Pin = USART1_RX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(USART1_RX_GPIO_PORT, &GPIO_InitStructure);

    //USART2
    GPIO_PinAFConfig(USART2_TX_GPIO_PORT, USART2_TX_GPIO_PIN, GPIO_AF_1);
    GPIO_InitStructure.GPIO_Pin = USART2_TX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(USART2_TX_GPIO_PORT, &GPIO_InitStructure);

    GPIO_PinAFConfig(USART2_RX_GPIO_PORT, USART2_RX_GPIO_PIN, GPIO_AF_1);
    GPIO_InitStructure.GPIO_Pin = USART2_RX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(USART2_RX_GPIO_PORT, &GPIO_InitStructure);

    //ADC
    GPIO_InitStructure.GPIO_Pin = ADC1_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; //模拟输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(ADC1_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ADC2_GPIO_PIN;
    GPIO_Init(ADC2_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ADC3_GPIO_PIN;
    GPIO_Init(ADC3_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = ADC4_GPIO_PIN;
    GPIO_Init(ADC4_GPIO_PORT, &GPIO_InitStructure);

    //DAC
    GPIO_InitStructure.GPIO_Pin = DAC_OUT1_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(DAC_OUT1_GPIO_PORT, &GPIO_InitStructure);

    //ADC放大倍数输出
    GPIO_InitStructure.GPIO_Pin = ADC_RATIOA1_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(ADC_RATIOA1_GPIO_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(ADC_RATIOA1_GPIO_PORT, ADC_RATIOA1_GPIO_PIN);

    GPIO_InitStructure.GPIO_Pin = ADC_RATIOA2_GPIO_PIN;
    GPIO_Init(ADC_RATIOA2_GPIO_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(ADC_RATIOA2_GPIO_PORT, ADC_RATIOA2_GPIO_PIN);

    GPIO_InitStructure.GPIO_Pin = ADC_RATIOB1_GPIO_PIN;
    GPIO_Init(ADC_RATIOB1_GPIO_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(ADC_RATIOB1_GPIO_PORT, ADC_RATIOB1_GPIO_PIN);

    // GPIO_InitStructure.GPIO_Pin = ADC_RATIOB2_GPIO_PIN;
    // GPIO_Init(ADC_RATIOB2_GPIO_PORT, &GPIO_InitStructure);
    // GPIO_ResetBits(ADC_RATIOB2_GPIO_PORT, ADC_RATIOB2_GPIO_PIN);

    //POWER输出
    GPIO_InitStructure.GPIO_Pin = POWER_EN_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(POWER_EN_GPIO_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(POWER_EN_GPIO_PORT, POWER_EN_GPIO_PIN);

    //POWER输入
    GPIO_InitStructure.GPIO_Pin = POWER_Key_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(POWER_Key_GPIO_PORT, &GPIO_InitStructure);

    //BAT输入
    GPIO_InitStructure.GPIO_Pin = BAT_CHRG_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(BAT_CHRG_GPIO_PORT, &GPIO_InitStructure);

    SYSCFG_EXTILineConfig(BAT_CHRG_EXTI_PORT, BAT_CHRG_EXTI_PIN);

    EXTI_InitStructure.EXTI_Line = BAT_CHRG_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = BAT_CHRG_EXTI_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    GPIO_InitStructure.GPIO_Pin = BAT_FAULT_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(BAT_FAULT_GPIO_PORT, &GPIO_InitStructure);

    SYSCFG_EXTILineConfig(BAT_FAULT_EXTI_PORT, EXTI_PortSourceGPIOB);

    EXTI_InitStructure.EXTI_Line = BAT_FAULT_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = BAT_FAULT_EXTI_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  ADC放大倍数控制函数
  * @param
  ADCNum 0第一个ADC, 1第二个ADC, 2第三个ADC, 3第四个ADC
  Ratio 放大倍数0~3
  */
void GpioADCRatio(uint8_t ADCNum, uint8_t Ratio)
{
    switch (ADCNum)
    {
    case 0:
        switch (Ratio)
        {
        case 0: //
            GPIO_ResetBits(ADC_RATIOA1_GPIO_PORT, ADC_RATIOA1_GPIO_PIN);
            GPIO_ResetBits(ADC_RATIOA2_GPIO_PORT, ADC_RATIOA2_GPIO_PIN);
            break;
        case 1: //
            GPIO_ResetBits(ADC_RATIOA1_GPIO_PORT, ADC_RATIOA1_GPIO_PIN);
            GPIO_SetBits(ADC_RATIOA2_GPIO_PORT, ADC_RATIOA2_GPIO_PIN);
            break;
        case 2: //
            GPIO_SetBits(ADC_RATIOA1_GPIO_PORT, ADC_RATIOA1_GPIO_PIN);
            GPIO_ResetBits(ADC_RATIOA2_GPIO_PORT, ADC_RATIOA2_GPIO_PIN);
            break;
        case 3: //
            GPIO_SetBits(ADC_RATIOA1_GPIO_PORT, ADC_RATIOA1_GPIO_PIN);
            GPIO_SetBits(ADC_RATIOA2_GPIO_PORT, ADC_RATIOA2_GPIO_PIN);
            break;
        default:
            GPIO_SetBits(ADC_RATIOA1_GPIO_PORT, ADC_RATIOA1_GPIO_PIN);
            GPIO_SetBits(ADC_RATIOA2_GPIO_PORT, ADC_RATIOA2_GPIO_PIN);
            break;
        }
        break;
    case 1:
        switch (Ratio)
        {
        case 0: //
            GPIO_ResetBits(ADC_RATIOB1_GPIO_PORT, ADC_RATIOB1_GPIO_PIN);
            break;
        case 1: //
            GPIO_SetBits(ADC_RATIOB1_GPIO_PORT, ADC_RATIOB1_GPIO_PIN);
            break;
        default:
            GPIO_SetBits(ADC_RATIOB1_GPIO_PORT, ADC_RATIOB1_GPIO_PIN);
            break;
        }
        break;
    default:
        break;
    }
}

/**
  * @brief  power电源保持控制函数
  * @param  Enable
  * @retval no
  */
void GpioPower(FunctionalState Enable) //推挽输出
{
    if (Enable)
        GPIO_SetBits(POWER_EN_GPIO_PORT, POWER_EN_GPIO_PIN);
    else
        GPIO_ResetBits(POWER_EN_GPIO_PORT, POWER_EN_GPIO_PIN);
}

/**
  * @brief 读取电源按键状态信息
  * @param  无
  * @retval 无   测量是上拉输入,低电平是按键按下状态
  */
void GpioStatusPower(void)//10ms周期执行
{
    static uint8_t UpCount = 0;
    static uint8_t DowCount = 0;

    if (GPIO_ReadInputDataBit(POWER_Key_GPIO_PORT, POWER_Key_GPIO_PIN))
    {
        DowCount = 0;
        if (UpCount < 200)//不能自加溢出,并且要大于下面判断值
        {
            UpCount++;
        }
    }
    else
    {
        if (UpCount > 20) //限制开机按下,抬起后再按下的时间间隔
        {
            DowCount++;
            if (DowCount > 100)
                PowerKeyStatus = 1;
        }
        else
        {
            UpCount = 0;
        }
    }
}

/**
  * @brief 读取电池充电状态信息
  * @param  无
  * @retval 无   测量是上拉输入,低电平是充电/故障状态
  */
void GpioStatusBAT(void)//gpio中断中执行
{
    if (GPIO_ReadInputDataBit(BAT_CHRG_GPIO_PORT, BAT_CHRG_GPIO_PIN))
    {
        BATStatus &= ~0x01;
    }
    else
    {
        BATStatus |= 0x01;
    }
    if (GPIO_ReadInputDataBit(BAT_FAULT_GPIO_PORT, BAT_FAULT_GPIO_PIN))
    {
        BATStatus &= ~0x02;
    }
    else
    {
        BATStatus |= 0x02;
    }
}