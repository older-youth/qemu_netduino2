#include <stdio.h>
#include <string.h>
#include "stm32f0xx.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_dma.h"
#include "stm32f0xx_usart.h"
#include "adcdma.h"
#include "crc.h"
#include "gpio.h"
#include "timer.h"
#include "uart.h"

FlagStatus UART1DMABusy = RESET;                  //存储测试板串口1状态，RESET不忙，SET忙
FlagStatus UART2DMABusy = RESET;                  //存储测试板串口2状态，RESET不忙，SET忙
FlagStatus UART1ReceFlag = RESET;                 //标记UART1是否收到数据 RESET 0未收到数据 SET 1收到数据
FlagStatus UART2ReceFlag = RESET;                 //标记UART2是否收到数据 RESET 0未收到数据 SET 1收到数据
uint8_t UART1ReadDataLen = 0;                     //记录USART1接收数组长度
uint8_t UART2ReadDataLen = 0;                     //记录USART2接收数组长度
// static char UART1SendData[USART1BUFFERNUM] = {0}; //‘\r\n’(0x13 0x10) 结尾
static char UART1ReceData[USART1BUFFERNUM] = {0}; //‘\r\n’(0x13 0x10) 结尾
// static char UART2SendData[USART2BUFFERNUM] = {0}; //‘\r\n’(0x13 0x10) 结尾
static char UART2ReceData[USART2BUFFERNUM] = {0}; //‘\r\n’(0x13 0x10) 结尾

/**
  * @brief  USART1DMA中断初始化
  * @param  None
  * @retval None
  */
static void USART1DMA_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    SYSCFG_DMAChannelRemapConfig(SYSCFG_DMARemap_USART1Tx, DISABLE); //Default DMA channel is mapped to the selected peripheral
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_3_IRQn;       //设置USART1 TX DMA中断参数
    NVIC_InitStructure.NVIC_IRQChannelPriority = 3;                  //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    SYSCFG_DMAChannelRemapConfig(SYSCFG_DMARemap_USART1Rx, DISABLE); //Default DMA channel is mapped to the selected peripheral
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_3_IRQn;       //设置USART1 RX DMA中断参数
    NVIC_InitStructure.NVIC_IRQChannelPriority = 3;                  //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  USART2DMA中断初始化
  * @param  None
  * @retval None
  */
static void USART2DMA_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    SYSCFG_DMAChannelRemapConfig(SYSCFG_DMARemap_USART2, DISABLE); //Default DMA channel is mapped to the selected peripheral
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_5_6_7_IRQn;     //设置USART2 TX DMA中断参数
    NVIC_InitStructure.NVIC_IRQChannelPriority = 3;                //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    SYSCFG_DMAChannelRemapConfig(SYSCFG_DMARemap_USART2, DISABLE); //Default DMA channel is mapped to the selected peripheral
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_5_6_7_IRQn;     //设置USART2 RX DMA中断参数
    NVIC_InitStructure.NVIC_IRQChannelPriority = 3;                //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  USART1中断初始化
  * @param  None
  * @retval None
  */
static void USART1_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; //设置USART1 RX中断参数
    NVIC_InitStructure.NVIC_IRQChannelPriority = 3;   //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  USART2中断初始化
  * @param  None
  * @retval None
  */
static void USART2_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; //设置USART2 RX中断参数
    NVIC_InitStructure.NVIC_IRQChannelPriority = 3;   //抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  USART1初始化
  * @param  no
  * @retval no
  */
void UART1Init(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    //USART1-TX
    DMA_DeInit(DMA1_Channel2);                                                           //复位DMA1,将外设 DMA1 Channel4的全部寄存器重设为缺省值
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;                    //DMA外设寄存器地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)NULL;                               //发送时添入 (uint32_t)UART1SendData;//DMA内存基地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                                   //DMA数据传输方向从内存到外设
    DMA_InitStructure.DMA_BufferSize = 0;//发送时添入 sizeof(UART1SendData) / sizeof(UART1SendData[0]); //DMA通道缓存大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                     //DNA外设地址自动增加失能
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                              //DNA内存地址自动增加使能
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;              //DMA外设传输字节宽度
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;                      //DMA内存传输字节宽度
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                                      //工作在循环缓存模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                                //DMA通道优先级
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                         //DMA通道没有设置为内存到内存传输
    DMA_Init(DMA1_Channel2, &DMA_InitStructure);                                         //根据以上参数初始化DMA1
    DMA_ITConfig(DMA1_Channel2, DMA_IT_TC | DMA_IT_TE, ENABLE);                          //使能指定通道中断
    DMA_Cmd(DMA1_Channel2, DISABLE);                                                     //每次发送数据时软件开启

    //USART1-RX
    DMA_DeInit(DMA1_Channel3);                                                           //复位DMA1,将外设 DMA1 Channel5的全部寄存器重设为缺省值
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;                    //DMA外设寄存器地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)UART1ReceData;                      //DMA内存基地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                                   //DMA数据传输方向从外设到内存
    DMA_InitStructure.DMA_BufferSize = sizeof(UART1ReceData) / sizeof(UART1ReceData[0]); //DMA通道缓存大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                     //DNA外设地址自动增加失能
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                              //DNA内存地址自动增加使能
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;              //DMA外设传输字节宽度
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;                      //DMA内存传输字节宽度
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                                      //工作在循环缓存模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                                //DMA通道优先级
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                         //DMA通道没有设置为内存到内存传输
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);                                         //根据以上参数初始化DMA1
    DMA_ITConfig(DMA1_Channel3, DMA_IT_TE, ENABLE);                                      //使能指定通道中断
    DMA_Cmd(DMA1_Channel3, DISABLE);//task开启后再启动

    USART1DMA_NVIC_Config(); //中断级别设置

    USART_InitStructure.USART_BaudRate = 19200;                                     //波特率 2400 9600 19200 57600 115200
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     //数据位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          //1个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;                             //无奇偶校验
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 //收发使能
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //RTS和CTS不使能
    USART_Init(USART1, &USART_InitStructure);

    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE); //接收空闲中断
    USART_ITConfig(USART1, USART_IT_PE, ENABLE);   //奇偶校验错误中断
    USART_ITConfig(USART1, USART_IT_ERR, ENABLE);  //包含了FE NE ORE
    //USART_ITConfig(USART1, USART_IT_FE, ENABLE);
    //USART_ITConfig(USART1, USART_IT_NE, ENABLE);
    //USART_ITConfig(USART2, USART_IT_ORE, ENABLE);

    USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE); //使能发送接收DMA请求
    //	USART_SetAddress(USART1, 0x5);						//设置USART节点的地址
    //	USART_WakeUpConfig(USART1, USART_WakeUp_AddressMark);			//地址标记唤醒
    USART1_NVIC_Config(); //中断级别设置
    USART_Cmd(USART1, ENABLE);
}

/**
  * @brief  USART2初始化
  * @param  no
  * @retval no
  */
void UART2Init(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    //USART2-TX
    DMA_DeInit(DMA1_Channel4);                                                           //复位DMA1,将外设 DMA1 Channel7的全部寄存器重设为缺省值
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;                    //DMA外设寄存器地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)NULL;                               //发送时添入 (uint32_t)UART2SendData;//DMA内存基地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                                   //DMA数据传输方向从内存到外设
    DMA_InitStructure.DMA_BufferSize = 0;//发送时添入 sizeof(UART2SendData) / sizeof(UART2SendData[0]); //DMA通道缓存大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                     //DNA外设地址自动增加失能
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                              //DNA内存地址自动增加使能
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;              //DMA外设传输字节宽度
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;                      //DMA内存传输字节宽度
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                                      //工作在循环缓存模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                                  //DMA通道优先级
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                         //DMA通道没有设置为内存到内存传输
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);                                         //根据以上参数初始化DMA1
    DMA_ITConfig(DMA1_Channel4, DMA_IT_TC | DMA_IT_TE, ENABLE);                          //使能指定通道中断
    DMA_Cmd(DMA1_Channel4, DISABLE);                                                     //每次发送数据时软件开启

    //USART2-RX
    DMA_DeInit(DMA1_Channel5);                                                           //复位DMA1,将外设 DMA1的全部寄存器重设为缺省值
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;                    //DMA外设寄存器地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)UART2ReceData;                      //DMA内存基地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                                   //DMA数据传输方向从外设到内存
    DMA_InitStructure.DMA_BufferSize = sizeof(UART2ReceData) / sizeof(UART2ReceData[0]); //DMA通道缓存大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                     //DNA外设地址自动增加失能
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                              //DNA内存地址自动增加使能
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;              //DMA外设传输字节宽度
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;                      //DMA内存传输字节宽度
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                                      //工作在循环缓存模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                                  //DMA通道优先级
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                         //DMA通道没有设置为内存到内存传输
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);                                         //根据以上参数初始化DMA1
    DMA_ITConfig(DMA1_Channel5, DMA_IT_TE, ENABLE);                                      //使能指定通道中断
    DMA_Cmd(DMA1_Channel5, DISABLE);//task开启后再启动

    USART2DMA_NVIC_Config(); //中断级别设置

    USART_InitStructure.USART_BaudRate = 19200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART2, &USART_InitStructure);

    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE); //接收空闲中断
    USART_ITConfig(USART2, USART_IT_PE, ENABLE);   //奇偶校验错误中断
    USART_ITConfig(USART2, USART_IT_ERR, ENABLE);  //包含了FE NE ORE
    //USART_ITConfig(USART2, USART_IT_FE, ENABLE);//帧检测错误中断
    //USART_ITConfig(USART2, USART_IT_NE, ENABLE);//噪音中断
    //USART_ITConfig(USART2, USART_IT_ORE, ENABLE);//接收溢出中断

    USART_DMACmd(USART2, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE); //
    USART2_NVIC_Config();                                            //中断级别设置
    USART_Cmd(USART2, ENABLE);
}

/**
  * @brief  串口数据处理
  * @param  UsartNumber 0串口1 1串口2
  * @retval None
  */
void USART_SystemVersionRead(UsartNumber_TypeDef UsartNumber)
{
    if (UsartNumber == UART_1 && UART1DMABusy == RESET)
    {
        memset(UART1SendData, 0, sizeof(UART1SendData)); //发送数组清零
#ifdef RS485
        sprintf(UART1SendData, "%s%u%s%s %s\r\n", USART_COLON, Flash_RemoteAddr, USART_SYSTEM, SYSTEM_VERSION, SOFTWARE_VER);
#else
        sprintf(UART1SendData, "%s%s %s\r\n", USART_SYSTEM, SYSTEM_VERSION, SOFTWARE_VER);
#endif
    }
    else if (UsartNumber == UART_2 && UART2DMABusy == RESET) //串口2
    {
        memset(UART2SendData, 0, sizeof(UART2SendData)); //发送数组清零
        sprintf(UART2SendData, "%s%s %s\r\n", USART_SYSTEM, SYSTEM_VERSION, SOFTWARE_VER);
    }
}

/**
  * @brief  串口数据处理
  * @param  UsartNumber 0串口1 1串口2
  * @retval None
  */
void USART_SystemStateRead(UsartNumber_TypeDef UsartNumber)
{
    if (UsartNumber == UART_1 && UART1DMABusy == RESET)
    {
        memset(UART1SendData, 0, sizeof(UART1SendData)); //发送数组清零
#ifdef RS485
        sprintf(UART1SendData, "%s%u%s%s %hu\r\n", USART_COLON, Flash_RemoteAddr, USART_SYSTEM, SYSTEM_STATE, SystemErrorStatus);
#else
        sprintf(UART1SendData, "%s%s %hu\r\n", USART_SYSTEM, SYSTEM_STATE, SystemErrorStatus);
#endif
    }
    else if (UsartNumber == UART_2 && UART2DMABusy == RESET) //串口2
    {
        memset(UART2SendData, 0, sizeof(UART2SendData)); //发送数组清零
        sprintf(UART2SendData, "%s%s %hu\r\n", USART_SYSTEM, SYSTEM_STATE, SystemErrorStatus);
    }
}

/**
  * @brief  串口数据处理
  * @param  UsartNumber 0串口1 1串口2
  * @retval None
  */
void USART_LightStateRead(UsartNumber_TypeDef UsartNumber)
{
    if (UsartNumber == UART_1 && UART1DMABusy == RESET)
    {
        memset(UART1SendData, 0, sizeof(UART1SendData)); //发送数组清零
#ifdef RS485
        sprintf(UART1SendData, "%s%u%s%s %hu_%hu\r\n", USART_COLON, Flash_RemoteAddr, USART_LIGHT, LIGHT_STATE, LightErrStatus, LightStepStatus);
#else
        sprintf(UART1SendData, "%s%s %hu_%hu\r\n", USART_LIGHT, LIGHT_STATE, LightErrStatus, LightStepStatus);
#endif
    }
    else if (UsartNumber == UART_2 && UART2DMABusy == RESET) //串口2
    {
        memset(UART2SendData, 0, sizeof(UART2SendData)); //发送数组清零
        sprintf(UART2SendData, "%s%s %hu_%hu\r\n", USART_LIGHT, LIGHT_STATE, LightErrStatus, LightStepStatus);
    }
}

/**
  * @brief  串口数据处理
  * @param  OnOff 0关闭OFF 1开启ON
  * @retval None
  */
void USART_LightPowerControl(PowerOnOff_TypeDef PowerOnOff)
{
    if (PowerOnOff == Power_Off)
    {
        if (LightStepStatus > 0 && LightStepStatus < 5)
        {
            LightStepStatus = 4; //停止
            KeyLightStep();
        }
    }
    else if (PowerOnOff == Power_On)
    {
        if (LightStepStatus == 0 || LightStepStatus == 5)
        {
            LightStepStatus = 0; //启动
            KeyLightStep();
        }
    }
}

/**
  * @brief  串口数据处理
  * @param  UsartNumber 0串口1 1串口2
  * @retval None
  */
void USART_LightInteRead(UsartNumber_TypeDef UsartNumber)
{
    if (UsartNumber == UART_1 && UART1DMABusy == RESET)
    {
        memset(UART1SendData, 0, sizeof(UART1SendData)); //发送数组清零
#ifdef RS485
        sprintf(UART1SendData, "%s%u%s%s %u\r\n", USART_COLON, Flash_RemoteAddr, USART_LIGHT, LIGHT_INTE, Flash_LightIntensity);
#else
        sprintf(UART1SendData, "%s%s %u\r\n", USART_LIGHT, LIGHT_INTE, Flash_LightIntensity);
#endif
    }
    else if (UsartNumber == UART_2 && UART2DMABusy == RESET) //串口2
    {
        memset(UART2SendData, 0, sizeof(UART2SendData)); //发送数组清零
        sprintf(UART2SendData, "%s%s %u\r\n", USART_LIGHT, LIGHT_INTE, Flash_LightIntensity);
    }
}

/**
  * @brief  串口数据处理
  * @param  UsartNumber 0串口1 1串口2
  * @retval None
  */
void USART_LightTimeRead(UsartNumber_TypeDef UsartNumber)
{
    if (UsartNumber == UART_1 && UART1DMABusy == RESET)
    {
        memset(UART1SendData, 0, sizeof(UART1SendData)); //发送数组清零
#ifdef RS485
        sprintf(UART1SendData, "%s%u%s%s %luh%us\r\n", USART_COLON, Flash_RemoteAddr, USART_LIGHT, LIGHT_TIME, Flash_LightTimeHour, Flash_LightTimeMinu);
#else
        sprintf(UART1SendData, "%s%s %luh%us\r\n", USART_LIGHT, LIGHT_TIME, Flash_LightTimeHour, Flash_LightTimeMinu);
#endif
    }
    else if (UsartNumber == UART_2 && UART2DMABusy == RESET) //串口2
    {
        memset(UART2SendData, 0, sizeof(UART2SendData)); //发送数组清零
        sprintf(UART2SendData, "%s%s %luh%us\r\n", USART_LIGHT, LIGHT_TIME, Flash_LightTimeHour, Flash_LightTimeMinu);
    }
}

/**
  * @brief  串口数据处理
  * @param  UsartNumber 0串口1 1串口2
  * @retval None
  */
void USART_LightADCRead(UsartNumber_TypeDef UsartNumber)
{
    if (UsartNumber == UART_1 && UART1DMABusy == RESET)
    {
        memset(UART1SendData, 0, sizeof(UART1SendData)); //发送数组清零
#ifdef RS485
        sprintf(UART1SendData, "%s%u%s%s %lu_%lu\r\n", USART_COLON, Flash_RemoteAddr, USART_LIGHT, LIGHT_ADC, ADCPercentConvert[0], ADCPercentConvert[1]);
#else
        sprintf(UART1SendData, "%s%s %lu_%lu\r\n", USART_LIGHT, LIGHT_ADC, ADCPercentConvert[0], ADCPercentConvert[1]);
#endif
    }
    else if (UsartNumber == UART_2 && UART2DMABusy == RESET) //串口2
    {
        memset(UART2SendData, 0, sizeof(UART2SendData)); //发送数组清零
        sprintf(UART2SendData, "%s%s %lu_%lu\r\n", USART_LIGHT, LIGHT_ADC, ADCPercentConvert[0], ADCPercentConvert[1]);
    }
}

/**
  * @brief  串口数据处理
  * @param  UsartNumber 0串口1 1串口2
  * @retval None
  */
void USART_FilterStateRead(UsartNumber_TypeDef UsartNumber)
{
    if (UsartNumber == UART_1 && UART1DMABusy == RESET)
    {
        memset(UART1SendData, 0, sizeof(UART1SendData)); //发送数组清零
#ifdef RS485
        sprintf(UART1SendData, "%s%u%s%s %hu_%hu_%hu_%hu\r\n", USART_COLON, Flash_RemoteAddr, USART_FILTER, FILTER_STATE, StepErrorStatus, StepRUNStatus, StepPositionCurrent, StepPositionTarget);
#else
        sprintf(UART1SendData, "%s%s %hu_%hu_%hu_%hu\r\n", USART_FILTER, FILTER_STATE, StepErrorStatus, StepRUNStatus, StepPositionCurrent, StepPositionTarget);
#endif
    }
    else if (UsartNumber == UART_2 && UART2DMABusy == RESET) //串口2
    {
        memset(UART2SendData, 0, sizeof(UART2SendData)); //发送数组清零
        sprintf(UART2SendData, "%s%s %hu_%hu_%hu_%hu\r\n", USART_FILTER, FILTER_STATE, StepErrorStatus, StepRUNStatus, StepPositionCurrent, StepPositionTarget);
    }
}

/**
  * @brief  串口数据处理
  * @param  UsartNumber 0串口1 1串口2
  * @retval None
  */
void USART_FilterDescriptionRead(UsartNumber_TypeDef UsartNumber, uint8_t FilterNumber)
{
    if (UsartNumber == UART_1 && UART1DMABusy == RESET)
    {
        memset(UART1SendData, 0, sizeof(UART1SendData)); //发送数组清零
#ifdef RS485
        sprintf(UART1SendData, "%s%u%s%s %hu_%s\r\n", USART_COLON, Flash_RemoteAddr, USART_FILTER, FILTER_DESCR, FilterNumber, Flash_FilterDescrip[Flash_FilterInd[FilterNumber - 1]]);
#else
        sprintf(UART1SendData, "%s%s %hu_%s\r\n", USART_FILTER, FILTER_DESCR, FilterNumber, Flash_FilterDescrip[Flash_FilterInd[FilterNumber - 1]]);
#endif
    }
    else if (UsartNumber == UART_2 && UART2DMABusy == RESET) //串口2
    {
        memset(UART2SendData, 0, sizeof(UART2SendData)); //发送数组清零
        sprintf(UART2SendData, "%s%s %hu_%s\r\n", USART_FILTER, FILTER_DESCR, FilterNumber, Flash_FilterDescrip[Flash_FilterInd[FilterNumber - 1]]);
    }
}

/**
  * @brief  串口数据处理
  * @param  UsartNumber 0串口1 1串口2
  * @retval None
  */
void USART_ShuterStateRead(UsartNumber_TypeDef UsartNumber)
{
    if (UsartNumber == UART_1 && UART1DMABusy == RESET)
    {
        memset(UART1SendData, 0, sizeof(UART1SendData)); //发送数组清零
#ifdef RS485
        sprintf(UART1SendData, "%s%u%s%s %hu_%hu_%hu\r\n", USART_COLON, Flash_RemoteAddr, USART_SHUTER, SHUTER_STATE, ShutErrStatus, ShutRunStatus, ShutterAMStatus);
#else
        sprintf(UART1SendData, "%s%s %hu_%hu_%hu\r\n", USART_SHUTER, SHUTER_STATE, ShutErrStatus, ShutRunStatus, ShutterAMStatus);
#endif
    }
    else if (UsartNumber == UART_2 && UART2DMABusy == RESET) //串口2
    {
        memset(UART2SendData, 0, sizeof(UART2SendData)); //发送数组清零
        sprintf(UART2SendData, "%s%s %hu_%hu_%hu\r\n", USART_SHUTER, SHUTER_STATE, ShutErrStatus, ShutRunStatus, ShutterAMStatus);
    }
}

/**
  * @brief  串口数据处理
  * @param  UsartNumber 0串口1 1串口2
  * @retval None
  */
void USART_ShuterTimeRead(UsartNumber_TypeDef UsartNumber)
{
    if (UsartNumber == UART_1 && UART1DMABusy == RESET)
    {
        memset(UART1SendData, 0, sizeof(UART1SendData)); //发送数组清零
#ifdef RS485
        sprintf(UART1SendData, "%s%u%s%s %lus\r\n", USART_COLON, Flash_RemoteAddr, USART_SHUTER, SHUTER_TIME, Flash_ShutterTime);
#else
        sprintf(UART1SendData, "%s%s %lus\r\n", USART_SHUTER, SHUTER_TIME, Flash_ShutterTime);
#endif
    }
    else if (UsartNumber == UART_2 && UART2DMABusy == RESET) //串口2
    {
        memset(UART2SendData, 0, sizeof(UART2SendData)); //发送数组清零
        sprintf(UART2SendData, "%s%s %lus\r\n", USART_SHUTER, SHUTER_TIME, Flash_ShutterTime);
    }
}

/**
  * @brief  串口数据处理
  * @param  UsartNumber 0串口1 1串口2
  * @retval None
  */
void USART_CommunicationStateRead(UsartNumber_TypeDef UsartNumber)
{
    if (UsartNumber == UART_1 && UART1DMABusy == RESET)
    {
        memset(UART1SendData, 0, sizeof(UART1SendData)); //发送数组清零
#ifdef RS485
        sprintf(UART1SendData, "%s%u%s%s %u_%u\r\n", USART_COLON, Flash_RemoteAddr, USART_COMM, COMM_STATE, Flash_RemoteStatus, Flash_RemoteAddr);
#else
        sprintf(UART1SendData, "%s%s %u_%u\r\n", USART_COMM, COMM_STATE, Flash_RemoteStatus, Flash_RemoteAddr);
#endif
    }
    else if (UsartNumber == UART_2 && UART2DMABusy == RESET) //串口2
    {
        memset(UART2SendData, 0, sizeof(UART2SendData)); //发送数组清零
        sprintf(UART2SendData, "%s%s %u_%u\r\n", USART_COMM, COMM_STATE, Flash_RemoteStatus, Flash_RemoteAddr);
    }
}

/**
  * @brief  串口1数据处理
  * @param  None
  * @retval None
  */
void USART1_SendDataProce(void)
{
    if (strlen(UART1SendData) && UART1DMABusy == RESET)
    {
        UART1DMABusy = SET;
#ifdef RS485
        STM32Delay_ms(1); //延时等待485芯片发送使能
#endif
        DMA_SetCurrDataCounter(DMA1_Channel4, strlen(UART1SendData)); //保证DMA传输不错位，失能才能写入
        DMA_Cmd(DMA1_Channel4, ENABLE);                               //每次发送数据时软件开启
    }
    else
    {
#ifdef RS485
        GpioRS485Send(DISABLE); //GPIO复位 RS485接收状态
#endif
    }
}

/**
  * @brief  串口1数据处理
  * @param  None
  * @retval None
  */
void USART1_ReadDataProce(void)
{
    if (UART1ReadDataLen > 4 && UART1ReceData[UART1ReadDataLen - 2] == 0x0D && UART1ReceData[UART1ReadDataLen - 1] == 0x0A) //‘\r\n’(0x13 0x10) 结尾
    {
        uint8_t AddrOffset = 0;
#ifdef RS485
        uint32_t temp = 0;
        char buf[10];

        sscanf(UART1ReceData, ":%d:", &temp);
        sprintf(buf, ":%d", Flash_RemoteAddr);
        AddrOffset = strlen(buf);
        if (UART1ReceData[0] == ':' && temp == Flash_RemoteAddr)
#endif
        {
            if (memcmp(UART1ReceData + AddrOffset, USART_SYSTEM, strlen(USART_SYSTEM)) == 0)
            {
                if (memcmp(UART1ReceData + AddrOffset + strlen(USART_SYSTEM), SYSTEM_VERSION, strlen(SYSTEM_VERSION)) == 0)
                {
                    USART_SystemVersionRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_SYSTEM), SYSTEM_STATE, strlen(SYSTEM_STATE)) == 0)
                {
                    USART_SystemStateRead(UART_1);
                }
            }
            else if (memcmp(UART1ReceData + AddrOffset, USART_LIGHT, strlen(USART_LIGHT)) == 0)
            {
                if (memcmp(UART1ReceData + AddrOffset + strlen(USART_LIGHT), LIGHT_STATE, strlen(LIGHT_STATE)) == 0)
                {
                    USART_LightStateRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_LIGHT), LIGHT_POWER_ON, strlen(LIGHT_POWER_ON)) == 0)
                {
                    USART_LightPowerControl(Power_On);
                    USART_LightStateRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_LIGHT), LIGHT_POWER_OFF, strlen(LIGHT_POWER_OFF)) == 0)
                {
                    USART_LightPowerControl(Power_Off);
                    USART_LightStateRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_LIGHT), LIGHT_INTE_SET, strlen(LIGHT_INTE_SET)) == 0)
                {
                    uint32_t temp = 0;
                    sscanf(UART1ReceData + AddrOffset + strlen(USART_LIGHT) + strlen(LIGHT_INTE_SET) + strlen(USART_SPACE), "%lu", &temp);
                    if (temp <= LIGHTINTMAX && temp != Flash_LightIntensity)
                    {
                        Flash_LightIntensity = (uint16_t)temp;
                        FLASH_DataWrite();
                    }
                    USART_LightInteRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_LIGHT), LIGHT_INTE_READ, strlen(LIGHT_INTE_READ)) == 0)
                {
                    USART_LightInteRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_LIGHT), LIGHT_TIME_READ, strlen(LIGHT_TIME_READ)) == 0)
                {
                    USART_LightTimeRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_LIGHT), LIGHT_TIME_RESET, strlen(LIGHT_TIME_RESET)) == 0)
                {
                    Flash_LightTimeHour = 0; //显示使用时间小时4字节
                    Flash_LightTimeMinu = 0; //显示使用时间分钟2字节
                    FLASH_DataWrite();
                    USART_LightTimeRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_LIGHT), LIGHT_ADC, strlen(LIGHT_ADC)) == 0)
                {
                    USART_LightADCRead(UART_1);
                }
            }
            else if (memcmp(UART1ReceData + AddrOffset, USART_FILTER, strlen(USART_FILTER)) == 0)
            {
                if (memcmp(UART1ReceData + AddrOffset + strlen(USART_FILTER), FILTER_STATE, strlen(FILTER_STATE)) == 0)
                {
                    USART_FilterStateRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_FILTER), FILTER_POSITION_SET, strlen(FILTER_POSITION_SET)) == 0)
                {
                    uint32_t temp = 0;
                    sscanf(UART1ReceData + AddrOffset + strlen(USART_FILTER) + strlen(FILTER_POSITION_SET) + strlen(USART_SPACE), "%lu", &temp);
                    if (temp > 0 && temp <= FILTERLOCMEMAX && temp != Flash_FilterLoc)
                    {
                        Flash_FilterLoc = (uint16_t)temp;
                        FLASH_DataWrite();
                    }
                    USART_FilterStateRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_FILTER), FILTER_DESCR_READ, strlen(FILTER_DESCR_READ)) == 0)
                {
                    uint32_t temp = 0;
                    sscanf(UART1ReceData + AddrOffset + strlen(USART_FILTER) + strlen(FILTER_DESCR_READ) + strlen(USART_SPACE), "%lu", &temp);
                    if (temp > 0 && temp <= FILTERLOCMEMAX)
                    {
                        USART_FilterDescriptionRead(UART_1, temp);
                    }
                }
            }
            else if (memcmp(UART1ReceData + AddrOffset, USART_SHUTER, strlen(USART_SHUTER)) == 0)
            {
                if (memcmp(UART1ReceData + AddrOffset + strlen(USART_SHUTER), SHUTER_STATE, strlen(SHUTER_STATE)) == 0)
                {
                    USART_ShuterStateRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_SHUTER), SHUTER_MA_OFF, strlen(SHUTER_MA_OFF)) == 0)
                {
                    ShutterAMStatus = 0;                               //快门手动自动状态 0手动 1自动
                    ShutterOnOffStatus = 0;                            //快门开关状态 0快门关 1快门开
                    GpioShutterC((FunctionalState)ShutterOnOffStatus); //控制快门
                    USART_ShuterStateRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_SHUTER), SHUTER_MA_ON, strlen(SHUTER_MA_ON)) == 0)
                {
                    ShutterAMStatus = 0;                               //快门手动自动状态 0手动 1自动
                    ShutterOnOffStatus = 1;                            //快门开关状态 0快门关 1快门开
                    GpioShutterC((FunctionalState)ShutterOnOffStatus); //控制快门
                    USART_ShuterStateRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_SHUTER), SHUTER_AU_ON, strlen(SHUTER_AU_ON)) == 0)
                {
                    ShutterAMStatus = 1;    //快门手动自动状态 0手动 1自动
                    ShutterOnOffStatus = 0; //快门开关状态 0快门关 1快门开
                    ShutTimScan();
                    USART_ShuterStateRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_SHUTER), SHUTER_AU_TIME, strlen(SHUTER_AU_TIME)) == 0)
                {
                    uint32_t temp = 0;
                    ShutterAMStatus = 1;    //快门手动自动状态 0手动 1自动
                    ShutterOnOffStatus = 0; //快门开关状态 0快门关 1快门开
                    sscanf(UART1ReceData + AddrOffset + strlen(USART_SHUTER) + strlen(SHUTER_AU_TIME) + strlen(USART_SPACE), "%lu", &temp);
                    if (temp > 0 && temp <= SHUTTERTIMEMAX && temp != Flash_ShutterTime)
                    {
                        Flash_ShutterTime = temp;
                        FLASH_DataWrite();
                    }
                    ShutTimScan();
                    USART_ShuterStateRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_SHUTER), SHUTER_TIME_WRITE, strlen(SHUTER_TIME_WRITE)) == 0)
                {
                    uint32_t temp = 0;
                    sscanf(UART1ReceData + AddrOffset + strlen(USART_SHUTER) + strlen(SHUTER_TIME_WRITE) + strlen(USART_SPACE), "%lu", &temp);
                    if (temp > 0 && temp <= SHUTTERTIMEMAX && temp != Flash_ShutterTime)
                    {
                        Flash_ShutterTime = temp;
                        FLASH_DataWrite();
                    }
                    USART_ShuterTimeRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_SHUTER), SHUTER_TIME_READ, strlen(SHUTER_TIME_READ)) == 0)
                {
                    USART_ShuterTimeRead(UART_1);
                }
            }
            else if (memcmp(UART1ReceData + AddrOffset, USART_COMM, strlen(USART_COMM)) == 0)
            {
                if (memcmp(UART1ReceData + AddrOffset + strlen(USART_COMM), SHUTER_STATE, strlen(SHUTER_STATE)) == 0)
                {
                    USART_CommunicationStateRead(UART_1);
                }
                else if (memcmp(UART1ReceData + AddrOffset + strlen(USART_COMM), COMM_SET, strlen(COMM_SET)) == 0)
                {
                    uint32_t tempRemote = 0;
                    uint32_t tempAddr = 0;
                    sscanf(UART1ReceData + AddrOffset + strlen(USART_COMM) + strlen(COMM_SET) + strlen(USART_SPACE), "%lu_%lu", &tempRemote, &tempAddr);
                    if ((tempRemote == 0 || tempRemote == 1) && tempAddr <= REMOTEADDRMAX && (tempRemote != Flash_RemoteStatus || tempAddr != Flash_RemoteAddr))
                    {
                        Flash_RemoteStatus = tempRemote;
                        Flash_RemoteAddr = tempAddr;
                        FLASH_DataWrite();
                    }
                    USART_CommunicationStateRead(UART_1);
                }
            }
        }
    }
    memset(UART1ReceData, 0, sizeof(UART1ReceData)); //接收数组清零
    //    USART_Cmd(USART1, ENABLE);

    USART1_SendDataProce();
}

/**
  * @brief  串口2数据处理
  * @param  None
  * @retval None
  */
void USART2_SendDataProce(void)
{
    if (strlen(UART2SendData) && UART2DMABusy == RESET)
    {
        UART2DMABusy = SET;
        DMA_SetCurrDataCounter(DMA1_Channel7, strlen(UART2SendData)); //保证DMA传输不错位，失能才能写入
        DMA_Cmd(DMA1_Channel7, ENABLE);                               //每次发送数据时软件开启
    }
}

/**
  * @brief  串口2数据处理
  * @param  None
  * @retval None
  */
void USART2_ReadDataProce(void)
{
    if (UART2ReadDataLen > 4 && UART2ReceData[UART2ReadDataLen - 2] == 0x0D && UART2ReceData[UART2ReadDataLen - 1] == 0x0A) //‘\r\n’(0x13 0x10) 结尾
    {
        if (memcmp(UART2ReceData, USART_SYSTEM, strlen(USART_SYSTEM)) == 0)
        {
            if (memcmp(UART2ReceData + strlen(USART_SYSTEM), SYSTEM_VERSION, strlen(SYSTEM_VERSION)) == 0)
            {
                USART_SystemVersionRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_SYSTEM), SYSTEM_STATE, strlen(SYSTEM_STATE)) == 0)
            {
                USART_SystemStateRead(UART_2);
            }
        }
        else if (memcmp(UART2ReceData, USART_LIGHT, strlen(USART_LIGHT)) == 0)
        {
            if (memcmp(UART2ReceData + strlen(USART_LIGHT), LIGHT_STATE, strlen(LIGHT_STATE)) == 0)
            {
                USART_LightStateRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_LIGHT), LIGHT_POWER_ON, strlen(LIGHT_POWER_ON)) == 0)
            {
                USART_LightPowerControl(Power_On);
                USART_LightStateRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_LIGHT), LIGHT_POWER_OFF, strlen(LIGHT_POWER_OFF)) == 0)
            {
                USART_LightPowerControl(Power_Off);
                USART_LightStateRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_LIGHT), LIGHT_INTE_SET, strlen(LIGHT_INTE_SET)) == 0)
            {
                uint32_t temp = 0;
                sscanf(UART2ReceData + strlen(USART_LIGHT) + strlen(LIGHT_INTE_SET) + strlen(USART_SPACE), "%lu", &temp);
                if (temp <= LIGHTINTMAX && temp != Flash_LightIntensity)
                {
                    Flash_LightIntensity = (uint16_t)temp;
                    FLASH_DataWrite();
                }
                USART_LightInteRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_LIGHT), LIGHT_INTE_READ, strlen(LIGHT_INTE_READ)) == 0)
            {
                USART_LightInteRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_LIGHT), LIGHT_TIME_READ, strlen(LIGHT_TIME_READ)) == 0)
            {
                USART_LightTimeRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_LIGHT), LIGHT_TIME_RESET, strlen(LIGHT_TIME_RESET)) == 0)
            {
                Flash_LightTimeHour = 0; //显示使用时间小时4字节
                Flash_LightTimeMinu = 0; //显示使用时间分钟2字节
                FLASH_DataWrite();
                USART_LightTimeRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_LIGHT), LIGHT_ADC, strlen(LIGHT_ADC)) == 0)
            {
                USART_LightADCRead(UART_2);
            }
        }
        else if (memcmp(UART2ReceData, USART_FILTER, strlen(USART_FILTER)) == 0)
        {
            if (memcmp(UART2ReceData + strlen(USART_FILTER), FILTER_STATE, strlen(FILTER_STATE)) == 0)
            {
                USART_FilterStateRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_FILTER), FILTER_POSITION_SET, strlen(FILTER_POSITION_SET)) == 0)
            {
                uint32_t temp = 0;
                sscanf(UART2ReceData + strlen(USART_FILTER) + strlen(FILTER_POSITION_SET) + strlen(USART_SPACE), "%lu", &temp);
                if (temp > 0 && temp <= FILTERLOCMEMAX && temp != Flash_FilterLoc)
                {
                    Flash_FilterLoc = (uint16_t)temp;
                    FLASH_DataWrite();
                }
                USART_FilterStateRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_FILTER), FILTER_DESCR_READ, strlen(FILTER_DESCR_READ)) == 0)
            {
                uint32_t temp = 0;
                sscanf(UART2ReceData + strlen(USART_FILTER) + strlen(FILTER_DESCR_READ) + strlen(USART_SPACE), "%lu", &temp);
                if (temp > 0 && temp <= FILTERLOCMEMAX)
                {
                    USART_FilterDescriptionRead(UART_2, temp);
                }
            }
        }
        else if (memcmp(UART2ReceData, USART_SHUTER, strlen(USART_SHUTER)) == 0)
        {
            if (memcmp(UART2ReceData + strlen(USART_SHUTER), SHUTER_STATE, strlen(SHUTER_STATE)) == 0)
            {
                USART_ShuterStateRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_SHUTER), SHUTER_MA_OFF, strlen(SHUTER_MA_OFF)) == 0)
            {
                ShutterAMStatus = 0;                               //快门手动自动状态 0手动 1自动
                ShutterOnOffStatus = 0;                            //快门开关状态 0快门关 1快门开
                GpioShutterC((FunctionalState)ShutterOnOffStatus); //控制快门
                USART_ShuterStateRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_SHUTER), SHUTER_MA_ON, strlen(SHUTER_MA_ON)) == 0)
            {
                ShutterAMStatus = 0;                               //快门手动自动状态 0手动 1自动
                ShutterOnOffStatus = 1;                            //快门开关状态 0快门关 1快门开
                GpioShutterC((FunctionalState)ShutterOnOffStatus); //控制快门
                USART_ShuterStateRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_SHUTER), SHUTER_AU_ON, strlen(SHUTER_AU_ON)) == 0)
            {
                ShutterAMStatus = 1;    //快门手动自动状态 0手动 1自动
                ShutterOnOffStatus = 0; //快门开关状态 0快门关 1快门开
                ShutTimScan();
                USART_ShuterStateRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_SHUTER), SHUTER_AU_TIME, strlen(SHUTER_AU_TIME)) == 0)
            {
                uint32_t temp = 0;
                ShutterAMStatus = 1;    //快门手动自动状态 0手动 1自动
                ShutterOnOffStatus = 0; //快门开关状态 0快门关 1快门开
                sscanf(UART2ReceData + strlen(USART_SHUTER) + strlen(SHUTER_AU_TIME) + strlen(USART_SPACE), "%lu", &temp);
                if (temp > 0 && temp <= SHUTTERTIMEMAX && temp != Flash_ShutterTime)
                {
                    Flash_ShutterTime = temp;
                    FLASH_DataWrite();
                }
                ShutTimScan();
                USART_ShuterStateRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_SHUTER), SHUTER_TIME_WRITE, strlen(SHUTER_TIME_WRITE)) == 0)
            {
                uint32_t temp = 0;
                sscanf(UART2ReceData + strlen(USART_SHUTER) + strlen(SHUTER_TIME_WRITE) + strlen(USART_SPACE), "%lu", &temp);
                if (temp > 0 && temp <= SHUTTERTIMEMAX && temp != Flash_ShutterTime)
                {
                    Flash_ShutterTime = temp;
                    FLASH_DataWrite();
                }
                USART_ShuterTimeRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_SHUTER), SHUTER_TIME_READ, strlen(SHUTER_TIME_READ)) == 0)
            {
                USART_ShuterTimeRead(UART_2);
            }
        }
        else if (memcmp(UART2ReceData, USART_COMM, strlen(USART_COMM)) == 0)
        {
            if (memcmp(UART2ReceData + strlen(USART_COMM), SHUTER_STATE, strlen(SHUTER_STATE)) == 0)
            {
                USART_CommunicationStateRead(UART_2);
            }
            else if (memcmp(UART2ReceData + strlen(USART_COMM), COMM_SET, strlen(COMM_SET)) == 0)
            {
                uint32_t tempRemote = 0;
                uint32_t tempAddr = 0;
                sscanf(UART2ReceData + strlen(USART_COMM) + strlen(COMM_SET) + strlen(USART_SPACE), "%lu_%lu", &tempRemote, &tempAddr);
                if ((tempRemote == 0 || tempRemote == 1) && tempAddr <= REMOTEADDRMAX && (tempRemote != Flash_RemoteStatus || tempAddr != Flash_RemoteAddr))
                {
                    Flash_RemoteStatus = tempRemote;
                    Flash_RemoteAddr = tempAddr;
                    FLASH_DataWrite();
                }
                USART_CommunicationStateRead(UART_2);
            }
        }
    }
    memset(UART2ReceData, 0, sizeof(UART2ReceData)); //接收数组清零
    //    USART_Cmd(USART2, ENABLE);

    USART2_SendDataProce();
}