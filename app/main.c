/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    22-September-2016
  * @brief   This file provides main program functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
// #include "adcdma.h"
// #include "clk.h"
// #include "gpio.h"
// #include "iwdg.h"
// #include "stmflash.h"
// #include "timer.h"
// #include "uart.h"
// #include "key.h"
// #include "FreeRTOS.h"
// #include "task.h"

// TaskHandle_t Task1_Handle, Task2_Handle;

// void task1_task()
// {
//   uint32_t testcount = xTaskGetTickCount();
//   while (testcount-xTaskGetTickCount() < 10)
//   {
//     testcount = xTaskGetTickCount();
//   }
  
//   while (1)
//   {
//     static uint8_t test1 = 0;

//     test1++;
//     vTaskDelay(1000);
//   }
// }

// void task2_task()
// {
//   uint32_t testcount = xTaskGetTickCount();
//   while (testcount-xTaskGetTickCount() < 20)
//   {
//     testcount = xTaskGetTickCount();
//   }
//   while (1)
//   {
//     static uint8_t test2 = 0;

//     test2++;
//     vTaskDelay(1500);
//   }
// }


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */ 
int main(void)
{
    /* Add your application code here
    C/C++ Compiler -> Preprocessor 定义STM32F10X_HD USE_STDPERIPH_DRIVER，定义在stm32f10x.h文件中使用，也可以在stm32f10x.h文件中定义。
    定义STM32F10X_HD是确定设备类型，高中低三个密度设备 在system_stm32f10x.c中查找可用定义
    定义USE_STDPERIPH_DRIVER是引用"stm32f10x_conf.h"外设配置文件
    HSE_VALUE 在stm32f10x.h 文件中定义，默认是8MHz根据实际情况更改，系统时钟频率是按照定义值计算的，不能测量实际值。
    在system_stm32f10x.c用的是SYSCLK_FREQ_72MHz，不是SYSCLK_FREQ_HSE，所以要在SetSysClockTo72函数中修改
    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMULL3);
    
    汇编语言启动system_stm32f10x.c文件中的SystemInit函数，再调用SetSysClock函数，再到main函数。SetSysClock函数启动HSE，
    根据HSE_VALUE定义设置系统时钟，如果外部高速晶振实际频率倍频后大于72MHz，不能进入main函数，出现不能调试现象。
    
    ARM_MATH_CM4这个就非常重要，必须要配置进去，否则在编译之后，会默认使用math.h的库函数，而不会用到硬件的FPU的。
    ARM_MATH_MATRIX_CHECK是库函数的参数检查开关，这里添加后就是打开。
    ARM_MATH_ROUNDING这个是库函数在运算是是否开启四舍五入的功能，这里添加，可以根据自己的需要进行配置。
    __ICCARM__、__CC_ARM、__GNUC__是不同编译器的编译配置宏定义，__ICCARM__是IAR环境,__CC_ARM是代表MDK开发环境，__GNUC__是gcc编译环境。
    
    Optimizations选择中等Medium时不能选择 common subexpression elimination
    */

    
    /* Add your application code here
    */
    // RccInit();
    // GpioInit();
    // ADCMDA_Init();//ADC初始化
    // DACMDA_Init();//DAC初始化
    // TimCyclic_Init();
    // STM32Delay_Init();
    // ShutTime_Init();
    // STMFLASH_Init();//STMFlash初始化
    // FLASH_DataRead();//读取Flash数据
    // UART1Init();
    // UART2Init();
    // ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    // USART_SystemStateRead(UART_1);
    // USART1_SendDataProce();
    // USART_SystemStateRead(UART_2);
    // USART2_SendDataProce();

    // xTaskCreate(task1_task, "Task1", 128, NULL, 2, &Task1_Handle);
    // xTaskCreate(task2_task, "Task2", 128, NULL, 3, &Task2_Handle);
    // vTaskStartScheduler();
    
    // WatchDogInit();
    
    /* Infinite loop */
    while (1)
    {
        // WatchDogFeed();
        // TimerProcess();//定时器扫描
        
//         if(FlashDataRefresh)
//         {
//             FlashDataRefresh = 0;
//             FLASH_DataWrite();
//         }
        
//         if(ADCConvertStatus)
//         {
//             ADCConvertStatus = 0;
//             ADCDMA_DataProce();//ADC数据处理
//             ADC_SoftwareStartConvCmd(ADC1, ENABLE);
//         }
    
//         if(UART1ReceFlag)
//         {
//             UART1ReceFlag = RESET;
//             USART1_ReadDataProce();
//         }
//         if(UART2ReceFlag)
//         {
//             UART2ReceFlag = RESET;
//             USART2_ReadDataProce();
//         }
        
        // if(Timer10msFlag)
        // {
        //     Timer10msFlag = 0;
        //     GpioStatusPower();
        // }
        
        // if(Timer200msFlag)
        // {
        //     Timer200msFlag = 0;
        // }
        
        // if(Timer1sFlag)
        // {
        //     Timer1sFlag = 0;
        // }
        
        // if(Timer1MinFlag)
        // {
        //     Timer1MinFlag = 0;
        // }
    }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
