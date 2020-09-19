#ifndef  __TIMER_H
#define  __TIMER_H


#ifdef __cplusplus
 extern "C" {
#endif 

#define   MAX_TIMER     7//定时器的最大个数 定时器0

extern uint32_t Tick_Num;
extern uint8_t Timer10msFlag;
extern uint8_t Timer200msFlag;
extern uint8_t Timer1sFlag;
extern uint8_t Timer1MinFlag;
extern volatile uint8_t STMDelayFlag;

typedef void(*TimerEventHandler_f)(uint8_t TimerId, void *arg);      //函数指针，u8和指针*arg是参数

typedef struct
{
    uint32_t  timeOut;
    uint32_t  ticks;
    TimerEventHandler_f handler;
    void  *arg;
}Timer_t;//定时器结构体

void TimCyclic_Init(void);
void TimCyclic_Process(void);
void STM32Delay_Init(void);
void STM32Delay_us(uint16_t us);
void STM32Delay_ms(uint16_t ms);

void AddTimer(uint8_t TimerID, uint32_t timeOut, TimerEventHandler_f  handler, void *arg); 
void DelTimer(uint8_t TimerID);         //删除定时器
void TimerProcess(void);                //主程序中需要处理的定时到函数

uint8_t IsTimer(uint8_t timerID);            //判断定时器是否存在
uint32_t Get_Tick(void);
uint32_t Get_TickGap(uint32_t Tick);


void ShutTime_Init(void);
void ShutTime_Start(void);
void ShutTime_Stop(void);

#ifdef __cplusplus
}
#endif

#endif



