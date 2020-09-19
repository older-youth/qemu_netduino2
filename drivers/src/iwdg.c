#include "stm32f0xx.h"
#include "stm32f0xx_iwdg.h"
#include "iwdg.h"

/**
  * @brief  独立看门狗初始化
  * @param  no
  * @retval no
  */
void WatchDogInit(void)
{
  IWDG_Enable(); //配置看门狗寄存器之前要先使能
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  IWDG_SetPrescaler(IWDG_Prescaler_256); //256分频，装载0x0FFF，26214s复位
  IWDG_SetReload(0x0FFF);
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
}

/**
  * @brief  喂独立看门狗
  * @param  no
  * @retval no
  */
void WatchDogFeed(void)
{
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  IWDG_ReloadCounter();
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
}