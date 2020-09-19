#include "stm32f0xx.h"
#include "stm32f0xx_rcc.h"
#include "clk.h"

/**
  * @brief  时钟初始化
  * @param  no
  * @retval no
  */
void RccInit(void)
{
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  // RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
  // RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);
  // RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
  // RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
  // RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FLITF, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SRAM, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART7, ENABLE);
  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART8, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE); //快门定时器
  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE);
  RCC_ADCCLKConfig(RCC_ADCCLK_PCLK_Div4);

  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE); //定时延时
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE); //10ms循环中断定时器
  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART4, ENABLE);
  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART5, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN, ENABLE);
  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_CRS, ENABLE);
  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE); //0-10V输出
  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_CEC, ENABLE);
}