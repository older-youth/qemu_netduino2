#ifndef __GPIO_H
#define __GPIO_H

#ifdef __cplusplus
extern "C"
{
#endif

//串口
//USART1
#define USART1_TX_GPIO_PORT GPIOA
#define USART1_TX_GPIO_PIN GPIO_Pin_9

#define USART1_RX_GPIO_PORT GPIOA
#define USART1_RX_GPIO_PIN GPIO_Pin_10

//USART2
#define USART2_TX_GPIO_PORT GPIOA
#define USART2_TX_GPIO_PIN GPIO_Pin_2

#define USART2_RX_GPIO_PORT GPIOA
#define USART2_RX_GPIO_PIN GPIO_Pin_3

//SPI未使用
//PSI1
#define SPI1_SCK_GPIO_PORT GPIOA
#define SPI1_SCK_GPIO_PIN GPIO_Pin_5

#define SPI1_MISO_GPIO_PORT GPIOA //主模式下接收数据
#define SPI1_MISO_GPIO_PIN GPIO_Pin_6

#define SPI1_MOSI_GPIO_PORT GPIOA //主模式下发送数据
#define SPI1_MOSI_GPIO_PIN GPIO_Pin_7

//PSI2
#define SPI2_SCK_GPIO_PORT GPIOB
#define SPI2_SCK_GPIO_PIN GPIO_Pin_13

#define SPI2_MISO_GPIO_PORT GPIOB //主模式下接收数据
#define SPI2_MISO_GPIO_PIN GPIO_Pin_14

#define SPI2_MOSI_GPIO_PORT GPIOB //主模式下发送数据
#define SPI2_MOSI_GPIO_PIN GPIO_Pin_15

#define SPI2_NRFCE_GPIO_PORT GPIOG //发收工作模式和待机工作模式切换
#define SPI2_NRFCE_GPIO_PIN GPIO_Pin_8

#define SPI2_NRFIRQ_GPIO_PORT GPIOG //NFR中断输出
#define SPI2_NRFIRQ_GPIO_PIN GPIO_Pin_6
#define SPI2_NRFIRQ_EXTI_PORTSOURCE GPIO_PortSourceGPIOG
#define SPI2_NRFIRQ_EXTI_PINSOURCE GPIO_PinSource6
#define SPI2_NRFIRQ_EXTI_LINE EXTI_Line6
#define SPI2_NRFIRQ_EXTI_IRQ EXTI9_5_IRQn

#define SPI2_NRFCS_GPIO_PORT GPIOG
#define SPI2_NRFCS_GPIO_PIN GPIO_Pin_7

#define SPI2_FLACS_GPIO_PORT GPIOB
#define SPI2_FLACS_GPIO_PIN GPIO_Pin_12

//SDIO SD卡未使用
#define SDIO_CMD_GPIO_PORT GPIOD
#define SDIO_CMD_GPIO_PIN GPIO_Pin_2

#define SDIO_CK_GPIO_PORT GPIOC
#define SDIO_CK_GPIO_PIN GPIO_Pin_12

#define SDIO_D0_GPIO_PORT GPIOC
#define SDIO_D0_GPIO_PIN GPIO_Pin_8

#define SDIO_D1_GPIO_PORT GPIOC
#define SDIO_D1_GPIO_PIN GPIO_Pin_9

#define SDIO_D2_GPIO_PORT GPIOC
#define SDIO_D2_GPIO_PIN GPIO_Pin_10

#define SDIO_D3_GPIO_PORT GPIOC
#define SDIO_D3_GPIO_PIN GPIO_Pin_11

//Timer
//STEP步进电机
#define STEP_OE_GPIO_PORT GPIOC //片使能 高电平使能
#define STEP_OE_GPIO_PIN GPIO_Pin_7

#define STEP_RST_GPIO_PORT GPIOC //复位 低电平复位
#define STEP_RST_GPIO_PIN GPIO_Pin_8

#define STEP_FR_GPIO_PORT GPIOC //方向 低电平正转
#define STEP_FR_GPIO_PIN GPIO_Pin_9

#define STEP_SP_GPIO_PORT GPIOA //步进
#define STEP_SP_GPIO_PIN GPIO_Pin_8

//ADC
#define ADC1_GPIO_PORT GPIOA //ADC_IN6
#define ADC1_GPIO_PIN GPIO_Pin_6

#define ADC2_GPIO_PORT GPIOA //ADC_IN7
#define ADC2_GPIO_PIN GPIO_Pin_7

#define ADC3_GPIO_PORT GPIOB //ADC_IN8
#define ADC3_GPIO_PIN GPIO_Pin_0

#define ADC4_GPIO_PORT GPIOB //ADC_IN9
#define ADC4_GPIO_PIN GPIO_Pin_1

//DAC
#define DAC_OUT1_GPIO_PORT GPIOA //DAC输出
#define DAC_OUT1_GPIO_PIN GPIO_Pin_4

//ADC放大倍数控制
#define ADC_RATIOA1_GPIO_PORT GPIOB //ADC1 第一级放大倍数
#define ADC_RATIOA1_GPIO_PIN GPIO_Pin_2

#define ADC_RATIOA2_GPIO_PORT GPIOB //ADC1 第二级放大倍数
#define ADC_RATIOA2_GPIO_PIN GPIO_Pin_10

#define ADC_RATIOB1_GPIO_PORT GPIOB //ADC2 第一级放大倍数
#define ADC_RATIOB1_GPIO_PIN GPIO_Pin_11

#define ADC_RATIOB2_GPIO_PORT GPIOD //ADC2 第二级放大倍数 未使用
#define ADC_RATIOB2_GPIO_PIN GPIO_Pin_3

//power_en
#define POWER_EN_GPIO_PORT GPIOB //按键电源保持输出
#define POWER_EN_GPIO_PIN GPIO_Pin_3

#define POWER_Key_GPIO_PORT GPIOA
#define POWER_Key_GPIO_PIN GPIO_Pin_15

//BAT
#define BAT_CHRG_GPIO_PORT GPIOB
#define BAT_CHRG_GPIO_PIN GPIO_Pin_4
#define BAT_CHRG_EXTI_PORT EXTI_PortSourceGPIOB
#define BAT_CHRG_EXTI_PIN EXTI_PinSource4
#define BAT_CHRG_EXTI_LINE EXTI_Line4
#define BAT_CHRG_EXTI_IRQ EXTI4_15_IRQn

#define BAT_FAULT_GPIO_PORT GPIOB
#define BAT_FAULT_GPIO_PIN GPIO_Pin_5
#define BAT_FAULT_EXTI_PORT EXTI_PortSourceGPIOB
#define BAT_FAULT_EXTI_PIN EXTI_PinSource5
#define BAT_FAULT_EXTI_LINE EXTI_Line5
#define BAT_FAULT_EXTI_IRQ EXTI4_15_IRQn

//测试按键
#define INT_Key0_GPIO_PORT GPIOE
#define INT_Key0_GPIO_PIN GPIO_Pin_4

    //IO口结构体
    // typedef struct //如果没有typedef就必须用struct Calendar_TypeDef来声明变量
    // {
    //     GPIO_TypeDef *GPIOx;
    //     uint16_t GPIO_Pin;
    // } GPIO_Port;

    //C中定义一个结构体类型要用typedef:
    //typedef struct [Student]//(如果没有typedef就必须用struct Student来声明变量)
    //{
    //int a;
    //}Stu;//Stu数据类型名，这里的Stu实际上就是struct Student的别名。Stu==struct Student
    //也可以不写Student

    //C++中可以与C中定义完全一样，也可以如下
    //struct Student//Student是结构体名 typedef不使用
    //{
    //    ...
    //}SU;//SU是结构体类型Student的一个变量

    extern uint8_t BATStatus;      //锂电池充电状态状态,0位1是正在充电,1位1是故障
    extern uint8_t PowerKeyStatus; //电源状态状态,1是按下关机,下板不处理关机逻辑,由上板来处理. 如果只有下板不存在开关机插电源就开机
    // extern uint8_t ShutRunStatus;              //快门运行状态，0已停止，1正在运行，2故障
    // extern uint8_t ShutErrStatus;              //快门错误状态保持，0已停止，0正常，1故障

    void GpioInit(void);
    void GpioADCRatio(uint8_t ADCNum, uint8_t Ratio);
    void GpioPower(FunctionalState Enable);
    void GpioStatusPower(void);
    void GpioStatusBAT(void);

#ifdef __cplusplus
}
#endif

#endif