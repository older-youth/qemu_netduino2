#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
extern "C"
{
#endif

    //#define RS485//使用RS485使能芯片前后要有延时

#define USART1BUFFERNUM 32 //USART1接收发送最大字节数
#define USART2BUFFERNUM 32 //USART2接收发送最大字节数

// #define SOFTWARE_VER "1.5.2" //版本号在makefile文件中定义

#define USART_END "\r\n"
#define USART_SPACE " "
#define USART_UNDERLIN "_"
#define USART_COLON ":"

#define USART_SYSTEM ":SYSTEM"
#define SYSTEM_VERSION ":VERSION"
#define SYSTEM_STATE ":STATE"

#ifdef CME_LOW
#define USART_LIGHT ":LIGH"
#else
#define USART_LIGHT ":LIGHT"
#endif
#define LIGHT_STATE ":STATE"
#define LIGHT_POWER_ON ":POWER_ON"
#define LIGHT_POWER_OFF ":POWER_OFF"
#define LIGHT_INTE ":INTE"
#define LIGHT_INTE_SET ":INTE_SET"
#define LIGHT_INTE_READ ":INTE_READ"
#define LIGHT_TIME ":TIME_LENG"
#define LIGHT_TIME_READ ":TIME_READ"
#define LIGHT_TIME_RESET ":TIME_RESET"
#define LIGHT_ADC ":ADC"

#ifdef CME_LOW
#define USART_FILTER ":FILTE"
#else
#define USART_FILTER ":FILTER"
#endif
#define FILTER_STATE ":STATE"
#define FILTER_POSITION_SET ":POSITION_SET"
#define FILTER_DESCR ":DESCR"
#define FILTER_DESCR_READ ":DESCR_READ"

#define USART_SHUTER ":SHUTER"
#define SHUTER_STATE ":STATE"
#define SHUTER_MA_ON ":MA_ON"
#define SHUTER_MA_OFF ":MA_OFF"
#define SHUTER_AU_ON ":AU_ON"
#define SHUTER_AU_TIME ":AU_TIME"
#define SHUTER_TIME ":TIME_LENG"
#define SHUTER_TIME_WRITE ":TIME_WRITE"
#define SHUTER_TIME_READ ":TIME_READ"

#define USART_COMM ":COMM"
#define COMM_STATE ":STATE"
#define COMM_SET ":SET"

    typedef union //如果没有typedef就必须用union CharFloat_TypeDef来声明变量
    {
        char c[4];
        uint32_t i;
    } CharUInt_TypeDef;

    typedef union //如果没有typedef就必须用union CharFloat_TypeDef来声明变量
    {
        char c[4];
        int i;
    } CharInt_TypeDef;

    typedef union //如果没有typedef就必须用union CharFloat_TypeDef来声明变量
    {
        char c[4];
        float f;
    } CharFloat_TypeDef;

    typedef enum
    {
        UART_1 = 0,
        UART_2,

        UART_Count
    } UsartNumber_TypeDef;

    typedef enum
    {
        Power_Off = 0,
        Power_On,

        Power_Count
    } PowerOnOff_TypeDef;

    extern FlagStatus UART1DMABusy;  //存储测试板串口1状态，RESET不忙，SET忙
    extern FlagStatus UART2DMABusy;  //存储测试板串口2状态，RESET不忙，SET忙
    extern FlagStatus UART1ReceFlag; //标记UART1是否收到数据 0未收到数据 1收到数据
    extern FlagStatus UART2ReceFlag; //标记UART2是否收到数据 0未收到数据 1收到数据
    extern uint8_t UART1ReadDataLen; //记录USART1接收数组长度
    extern uint8_t UART2ReadDataLen; //记录USART2接收数组长度

    void UART1Init(void);
    void UART2Init(void);
    void USART_SystemStateRead(UsartNumber_TypeDef UsartNumber);
    void USART_LightStateRead(UsartNumber_TypeDef UsartNumber);
    void USART_FilterStateRead(UsartNumber_TypeDef UsartNumber);
    void USART1_SendDataProce(void);
    void USART2_SendDataProce(void);
    void USART1_ReadDataProce(void);
    void USART2_ReadDataProce(void);

#ifdef __cplusplus
}
#endif

#endif