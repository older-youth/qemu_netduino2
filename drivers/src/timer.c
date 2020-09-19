#include "stm32f0xx.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_misc.h"
#include "iwdg.h"
#include "timer.h"

uint32_t Tick_Num = 0;
uint8_t Timer10msFlag = 0;
uint8_t Timer200msFlag = 0;
uint8_t Timer1sFlag = 0;
uint8_t Timer1MinFlag = 0;
volatile uint8_t STMDelayFlag = 0;
static Timer_t timerArray[MAX_TIMER] = {0};

/*
* Function name : Tim17Init
* Input param 	: None
* Return 		: None
* 定时器溢出时间计算: (分频值/Fcpu)*计数值 Fcpu 48M
*/
void TimCyclic_Init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

    NVIC_InitStructure.NVIC_IRQChannel = TIM17_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_TimeBaseInitStruct.TIM_Prescaler = 127; //48MHz时128分频   72MHz时192分频
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = 0x0EA6; //128预分频 3750计数 10ms中断
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;

    TIM_ITConfig(TIM17, TIM_IT_Update, DISABLE); //设置中断
    TIM_TimeBaseInit(TIM17, &TIM_TimeBaseInitStruct);
    TIM_ClearFlag(TIM17, TIM_IT_Update);
    TIM_ITConfig(TIM17, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM17, ENABLE);
}

/**
  * @brief  TimCyclic_Proc 中断循环处理
  * @param  None
  * @retval None
  */
void TimCyclic_Process(void)
{
    static uint16_t Tick_timer = 0;

    Timer10msFlag = 1;
    Tick_Num++;
    Tick_timer++;
    if (Tick_timer % 20 == 0)
    {
        Timer200msFlag = 1;
    }
    if (Tick_timer % 100 == 0)
    {
        Timer1sFlag = 1;
    }
    if (Tick_timer >= 6000)
    {
        Tick_timer = 0;
        Timer1MinFlag = 1;
    }
}

/**
  * @brief  STM32Delay函数初始化，使能中断
  * @param  None
  * @retval None
  */
void STM32Delay_Init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = TIM16_IRQn; //延时定时器
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
* Function name : STM32Delay_us
* Description 	: 延时 晶振频率: 48M
* Input param 	: us:1-65535us
* Return        : None
*/
void STM32Delay_us(uint16_t us)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

    STMDelayFlag = 0;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 47; //48MHz时48预分频     72MHz时72分频
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = us;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;

    TIM_ITConfig(TIM16, TIM_IT_Update, DISABLE); //设置中断
    TIM_TimeBaseInit(TIM16, &TIM_TimeBaseInitStruct);
    TIM_ClearFlag(TIM16, TIM_IT_Update);
    TIM_ITConfig(TIM16, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM16, ENABLE);

    while (!STMDelayFlag)
        ;
}

/**************************************************
* Function name : STM32Delay_ms
* Description 	: 延时 晶振频率: 48M
* Input param 	: 48M us:1-65535ms; 72M us:1-32767ms
* Return        : None
**************************************************/
void STM32Delay_ms(uint16_t ms)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

    STMDelayFlag = 0;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 0xBB7F; //48MHz时48000预分频0xBB7F     72MHz时36000预分频0x8C9F
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    // if (ms > 32767) //72MHz,36K分频，2000计数/s
    //     TIM_TimeBaseInitStruct.TIM_Period = 65535;
    // else
    //     TIM_TimeBaseInitStruct.TIM_Period = ms * 2;
    TIM_TimeBaseInitStruct.TIM_Period = ms;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
    
    TIM_ITConfig(TIM16, TIM_IT_Update, DISABLE); //设置中断
    TIM_TimeBaseInit(TIM16, &TIM_TimeBaseInitStruct);
    TIM_ClearFlag(TIM16, TIM_IT_Update);
    TIM_ITConfig(TIM16, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM16, ENABLE);

    while (!STMDelayFlag)
    {
        WatchDogFeed();
    }
}

/*
* Function name : Get_Tick
* Description 	: 获取系统Tick值
* Input param 	: None
* Return        : 返回定时器计数周期值
* See also      : None
*/
uint32_t Get_Tick(void)
{
    uint32_t tmp;
    tmp = Tick_Num;
    return tmp;
}

/*
* Function name : Get_TickGap
* Description 	: 获取定时器Tick间隔值
* Input param 	: 
Tick            :记录的原始数值
* Return        : 返回定时器计数间隔值
* See also      : None
*/
uint32_t Get_TickGap(uint32_t Tick)
{
    uint32_t curruntimer = 0;

    if (Tick_Num >= Tick)
    {
        curruntimer = Tick_Num - Tick;
    }
    else //ticks值溢出
    {
        curruntimer = 0xFFFFFFFF - Tick + Tick_Num + 0x01;
    }
    return curruntimer;
}

/*
* Function name : AddTimer
* Description 	:增加定时器
* Input param 	:
TimerID         :ID号0-3
timeOut         :超时处理时间 以定时器中断周期为基准时间 成倍增加
handler         :定时器时间到执行函数
*arg            :定时器执行函数的参数
* Return        : None
*/
void AddTimer(uint8_t TimerID, uint32_t timeOut, TimerEventHandler_f handler, void *arg)
{
    if (TimerID > (MAX_TIMER - 1))
    {
        return;
    }
    timerArray[TimerID].timeOut = timeOut;
    timerArray[TimerID].ticks = Get_Tick();
    timerArray[TimerID].handler = handler;
    timerArray[TimerID].arg = arg;
}

/*
* Function name : DelTimer
* Description 	:删除定时器
* Input param 	:
TimerID         :ID号0-3
* Return        : None
*/
void DelTimer(uint8_t TimerID)
{
    timerArray[TimerID].timeOut = 0;
    timerArray[TimerID].ticks = 0;
    timerArray[TimerID].handler = 0;
    timerArray[TimerID].arg = 0;
}

/*
* Function name : IsTimer
* Description 	:判断定时器是否非空
* Input param 	:
TimerID         :ID号0-3
* Return        : None
*/
uint8_t IsTimer(uint8_t TimerID)
{
    if (timerArray[TimerID].handler == 0)
    {
        return 0;
    }
    return 1;
}

/*
* Function name : TimerProcess
* Description 	:在main函数中循环判断定时器是否延时时间到
* Input param 	:
* Return        : None
*/
void TimerProcess(void)
{
    uint8_t i;

    for (i = 0; i < MAX_TIMER; i++)
    {
        if (timerArray[i].handler == 0)
        {
            continue;
        }
        else
        {
            if (Get_TickGap(timerArray[i].ticks) >= timerArray[i].timeOut)
            {
                timerArray[i].ticks = Get_Tick(); //定时时间到，更新定时器
                timerArray[i].handler(i, timerArray[i].arg);
            }
        }
    }
}

// /**
//   * @brief  快门硬件定时器函数初始化，使能中断
//   * @param  None
//   * @retval None
//   */
// void ShutTime_Init(void)
// {
//     NVIC_InitTypeDef NVIC_InitStructure;
//     TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

//     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//     NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
//     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
//     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
//     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//     NVIC_Init(&NVIC_InitStructure);

//     TIM_TimeBaseInitStruct.TIM_Prescaler = 0x8C9F; //36000预分频
//     TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
//     TIM_TimeBaseInitStruct.TIM_Period = 2000; //1s周期
//     TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
//     TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
//     TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);

//     //    TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);//设置中断
//     TIM_ClearFlag(TIM2, TIM_IT_Update);
//     TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

//     TIM_SelectOnePulseMode(TIM2, TIM_OPMode_Single);
// }

// /**
//   * @brief  快门硬件定时器启动
//   * @param  None
//   * @retval None
//   */
// void ShutTime_Start(void)
// {
//     TIM_Cmd(TIM2, ENABLE);
// }

// /**
//   * @brief  快门硬件定时器停止
//   * @param  None
//   * @retval None
//   */
// void ShutTime_Stop(void)
// {
//     TIM_Cmd(TIM2, DISABLE);
// }