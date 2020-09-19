#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "stm32f10x.h"
#include "adcdma.h"
#include "gpio.h"
#include "lcd.h"
#include "stmflash.h"
#include "step.h"
#include "timer.h"
#include "uart.h"
#include "key.h"

//点灯顺控状态 0未操作 1第一个继电器吸合 2第二个继电器吸合 3断开第二个继电器开启风扇 4第一个继电器断开风扇继续运行
uint8_t SystemErrorStatus = 0;//标记系统运行状态，0位步进电机故障 1位快门故障 2位继电器故障 3位风扇故障 4位机箱门故障 5位温度故障
uint8_t LightStepStatus = 0;//标记点灯程序是否正在进行，大于0进行
uint8_t LightErrStatus = 0;//标记点灯关键设备故障，0正常 1故障
uint8_t ShutterAMStatus = 0;//快门手动自动状态 0手动 1自动
uint8_t ShutterOnOffStatus = 0;//快门开关状态 0快门关 1快门开

uint16_t VAR_LightIntensity = 0;//灯光强度设定值 2个字节，1个16位
uint32_t VAR_ShutterTime = 0;//快门时间间隔4字节，1个32位，单位s
uint16_t VAR_FilterLoc = 0;//滤光片位置2个字节，1个16位
uint16_t VAR_FilterInd = 0;//当前位置滤光片位置对应的索引号
uint16_t VAR_LightStatus = 0;//光强度控制状态 0比例控制输出 1手动控制输出
uint32_t VAR_LightMini = 0;//光强度测量值百分化最小值
uint32_t VAR_LightMaxi = 0;///光强度测量值百分化最大值
uint16_t VAR_RemoteAdd = 0;//设备远程通讯地址


KeyInPut_TypeDef KeyInPutStat;
KeyEventQueue_TypeDef KeyEvenQueue;//按键事件列队
KeyStatus_TypeDef KeyStatus = STATE_IDLE;//按键状态机
KeyScanPro_TypeDef KeyScanPro[MAX_KEY_COUNT] = {0};
typedef  void(*pfKeyScanProcess)(void);//状态机扫描事件函数指针
pfKeyScanProcess KeyStatusArray[STATE_COUNT][EVENT_COUNT];//获取状态机函数

/**
  * @brief  向事件结构体增加事件
  * @param
  KeyEvenQueue 消息队列结构体指针
  KeyEvent 添加事件编号
  * @retval 1添加成功,0列队已满
  */
static uint8_t AddEventToQueue(KeyEventQueue_TypeDef *KeyEvenQueue, KeyEvent_TypeDef KeyEvent)
{
    if(KeyEvenQueue->ReadPos == (KeyEvenQueue->WritePos+1)%MAX_KEY_EVENT_CNT)//队列已满
        return 0;
    KeyEvenQueue->KeyEventQueue[KeyEvenQueue->WritePos] = KeyEvent;
    KeyEvenQueue->WritePos = (KeyEvenQueue->WritePos+1)%MAX_KEY_EVENT_CNT;   //按键列队写入标记加1，为下次写入做好准备
    return 1;
}

/**
  * @brief  读取事件结构体事件
  * @param
  KeyEvenQueue 消息队列结构体指针
  KeyEvent 读取事件编号，指针返回数据
  * @retval 1读取成功,0读取失败列队空
  */
static uint8_t ReadEventFromQueue(KeyEventQueue_TypeDef *KeyEvenQueue, KeyEvent_TypeDef *pKeyEvent)
{
    if(KeyEvenQueue->ReadPos == KeyEvenQueue->WritePos)
        return 0;//列队空
    *pKeyEvent = KeyEvenQueue->KeyEventQueue[KeyEvenQueue->ReadPos];
    KeyEvenQueue->ReadPos = (KeyEvenQueue->ReadPos+1)%MAX_KEY_EVENT_CNT;   //按键列队读取标记加1，为下次读取做好准备
    return 1;
}

/**
  * @brief  LCD界面控制函数
  * @param  none
  * @retval none
  */
void LCD_DisplayIdle(void)
{
    LCD_DisplayFilter(Flash_FilterLoc, Flash_FilterDescrip[Flash_FilterInd[Flash_FilterLoc-1]]);
    LCD_DisplayLightIn(Flash_LightIntensity);//ADCLUXPercent[1]
    LCD_DisplayShutter(ShutterAMStatus, Flash_ShutterTime);
    LCD_DisplayRemote(Flash_RemoteStatus, Flash_RemoteAddr);
    LCD_CursorBlinkOff();//关闭字符闪烁状态
}

/**
  * @brief  按键LED控制函数，在状态函数中执行
  * @param  none
  * @retval none
  */
static void KeyStatusLED(void)
{
    for(uint8_t i=KEYNUMBERSTAR; i<KEYNUMBERSTOP; i++)
    {
        GpioLEDC(i, DISABLE);//按键指示灯从4开始至9
    }
    switch(KeyStatus)
    {
	case STATE_IDLE:
	    break;
    case STATE_SHUTTER_TI:
        GpioLEDC(KEYNUMBERSTAR + 1, ENABLE);//快门时间设置状态
        break;
    case STATE_FILTER_LO:
        GpioLEDC(KEYNUMBERSTAR + 2, ENABLE);//滤光片位置设置状态
        break;
    case STATE_FILTER_TY:
        GpioLEDC(KEYNUMBERSTAR + 2, ENABLE);//滤光片型号设置状态
        break;
    case STATE_LIGHT_LI:
        GpioLEDC(KEYNUMBERSTAR + 3, ENABLE);//光强强度设置状态
        break;
    case STATE_LIGHT_AU:
        GpioLEDC(KEYNUMBERSTAR + 0, ENABLE);//光强强度控制设置手自动
        break;
    case STATE_LIGHT_Min:
        GpioLEDC(KEYNUMBERSTAR + 0, ENABLE);//光强强度控制设置最小值
        break;
    case STATE_LIGHT_Max:
        GpioLEDC(KEYNUMBERSTAR + 0, ENABLE);//光强强度控制设置最大值
        break;
    case STATE_REMOTE_ADD:
        GpioLEDC(KEYNUMBERSTAR + 0, ENABLE);//远程地址设置状态
        break;
    case STATE_LIGHT_TI:
        GpioLEDC(KEYNUMBERSTAR + 3, ENABLE);//灯泡使用时间查看状态
        break;
    case STATE_REMOTE_SE:
        GpioLEDC(KEYNUMBERSTAR + 4, ENABLE);//远程就地控制设置状态
        break;
    case STATE_ERROR_LO:
        GpioLEDC(KEYNUMBERSTAR + 5, ENABLE);//故障查看状态
        break;
    default:
        break;
    }
}

/**
  * @brief  输入数据左右移位函数，进入设置状态时被调用
  * @param  none
  * @retval none
  */
static void KeyInPutInit(void)
{
    switch(KeyStatus)
    {
    case STATE_SHUTTER_TI:
        KeyInPutStat.KeyInPutStart = KEYINPUTSTART_SHUTTER;
        KeyInPutStat.KeyInPutEnd = KEYINPUTEND_SHUTTER;
        KeyInPutStat.KeyInPutLine = KEYINPUTLINE_SHUTTER;//快门时间设置状态
        break;
    case STATE_FILTER_LO://不能左右移位 但开启闪烁
        KeyInPutStat.KeyInPutStart = KEYINPUTSTART_FILTERLO;
        KeyInPutStat.KeyInPutEnd = KEYINPUTEND_FILTERLO;
        KeyInPutStat.KeyInPutLine = KEYINPUTLINE_FILTERLO;//滤光片位置设置状态
        break;
    case STATE_FILTER_TY://不能左右移位 但开启闪烁
        KeyInPutStat.KeyInPutStart = KEYINPUTSTART_FILTERTY;
        KeyInPutStat.KeyInPutEnd = KEYINPUTEND_FILTERTY;
        KeyInPutStat.KeyInPutLine = KEYINPUTLINE_FILTERTY;//滤光片型号设置状态
        break;
    case STATE_LIGHT_LI:
        KeyInPutStat.KeyInPutStart = KEYINPUTSTART_LIGHT;
        KeyInPutStat.KeyInPutEnd = KEYINPUTEND_LIGHT;
        KeyInPutStat.KeyInPutLine = KEYINPUTLINE_LIGHT;//光强强度设置状态
        break;
    case STATE_LIGHT_AU://不能左右移位 但开启闪烁
        KeyInPutStat.KeyInPutStart = KEYINPUTSTART_LIGHTAU;
        KeyInPutStat.KeyInPutEnd = KEYINPUTEND_LIGHTAU;
        KeyInPutStat.KeyInPutLine = KEYINPUTLINE_LIGHTAU;//光强强度控制设置手自动
        break;
    case STATE_LIGHT_Min:
        KeyInPutStat.KeyInPutStart = KEYINPUTSTART_LIGHTMIN;
        KeyInPutStat.KeyInPutEnd = KEYINPUTEND_LIGHTMIN;
        KeyInPutStat.KeyInPutLine = KEYINPUTLINE_LIGHTMIN;//光强强度控制设置最小值
        break;
    case STATE_LIGHT_Max:
        KeyInPutStat.KeyInPutStart = KEYINPUTSTART_LIGHTMAX;
        KeyInPutStat.KeyInPutEnd = KEYINPUTEND_LIGHTMAX;
        KeyInPutStat.KeyInPutLine = KEYINPUTLINE_LIGHTMAX;//光强强度控制设置最大值
        break;
    case STATE_REMOTE_ADD:
        KeyInPutStat.KeyInPutStart = KEYINPUTSTART_REMOTEADD;
        KeyInPutStat.KeyInPutEnd = KEYINPUTEND_REMOTEADD;
        KeyInPutStat.KeyInPutLine = KEYINPUTLINE_REMOTEADD;//远程地址设置状态
        break;
    default:
        KeyInPutStat.KeyInPutStart = 0;
        KeyInPutStat.KeyInPutEnd = 0;
        KeyInPutStat.KeyInPutLine = 0;
        break;
    }
    KeyInPutStat.KeyInPutCurrent = KeyInPutStat.KeyInPutEnd;
    if(KeyInPutStat.KeyInPutCurrent || KeyInPutStat.KeyInPutLine)//所有数据不从0位显示
        LCD_CursorBlinkOn(KeyInPutStat.KeyInPutLine, KeyInPutStat.KeyInPutCurrent);
}

void KeyLightStep(void);
static void Proc_KEYUPDOWN(uint8_t Direction);

/**
  * @brief  定时器0 状态机超时处理函数
  * @param
  TimerId 定时器ID
  * @retval none
  */
static void TimerScanStatus(uint8_t TimerId, void *arg)
{
    DelTimer(TimerId);  
    AddEventToQueue(&KeyEvenQueue, KEY_TIME_EVENT);    
}

/**
  * @brief  定时器1 点灯逻辑定时器
  * @param
  TimerId 定时器ID
  * @retval none
  */
static void TimerScanLight(uint8_t TimerId, void *arg)
{
    DelTimer(TimerId);//定时器函数指针增加定时器，所以先删除
    KeyLightStep();
}

/**
  * @brief  定时器2 状态机长按事件快速加减
  * @param
  TimerId 定时器ID
  * @retval none
  */
static void TimerScanUpDown(uint8_t TimerId, void *arg)
{
    if(KeyStatus == STATE_SHUTTER_TI || KeyStatus == STATE_FILTER_LO || KeyStatus == STATE_FILTER_TY || KeyStatus == STATE_LIGHT_LI || \
        KeyStatus == STATE_LIGHT_AU || KeyStatus == STATE_LIGHT_Min || KeyStatus == STATE_LIGHT_Max || KeyStatus == STATE_REMOTE_ADD)
    {
        if(!GPIO_ReadInputDataBit(GPIOPortKEY[MAX_KEY_COUNT - 1].GPIOx, GPIOPortKEY[MAX_KEY_COUNT - 1].GPIO_Pin))
        {
            Proc_KEYUPDOWN(1);
            AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
        }
        else if(!GPIO_ReadInputDataBit(GPIOPortKEY[MAX_KEY_COUNT - 2].GPIOx, GPIOPortKEY[MAX_KEY_COUNT - 2].GPIO_Pin))
        {
            Proc_KEYUPDOWN(0);
            AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
        }
        else
        {
            KeyStatusLED();
            DelTimer(TimerId);//删除上下键长按所增减的快速增加数据的定时器
        }
    }
    else
    {
        KeyStatusLED();
        DelTimer(TimerId);
    }
}

/**
  * @brief  定时器4 刷新故障显示
  * @param
  TimerId 定时器ID
  * @retval none
  */
static void TimerScanErroUpdata(uint8_t TimerId, void *arg)
{
    if(KeyStatus == STATE_ERROR_LO)
    {
        LCD_DisplayErrorStatus(SystemErrorStatus);//更新LCD显示
    }
    else
        DelTimer(TimerId);
}

/**
  * @brief  定时器5 刷新光照度ADC测量
  * @param
  TimerId 定时器ID
  * @retval none
  */
static void TimerScanLightADCUpdata(uint8_t TimerId, void *arg)
{
    if(KeyStatus == STATE_LIGHT_Min)
    {
        LCD_DisplaySetLightMinStatus(ADCPercentConvert[1], VAR_LightMini);//更新LCD显示
        LCD_CursorBlinkOn(KeyInPutStat.KeyInPutLine, KeyInPutStat.KeyInPutCurrent);//开启光标光标闪烁
    }
    else if(KeyStatus == STATE_LIGHT_Max)
    {
        LCD_DisplaySetLightMaxStatus(ADCPercentConvert[1], VAR_LightMaxi);//更新LCD显示
        LCD_CursorBlinkOn(KeyInPutStat.KeyInPutLine, KeyInPutStat.KeyInPutCurrent);//开启光标光标闪烁
    }
    else
        DelTimer(TimerId);
}

/**
  * @brief  点灯顺控函数
  * @param none
  * @retval none
  */
void KeyLightStep(void)
{
    switch(LightStepStatus)
    {
    case 0:
        LightStepStatus = 1;
        GpioRealayC(0, ENABLE);//继电器1吸合
        AddTimer(1, EXPIRE_LightRelay1, TimerScanLight, (void*) &KeyStatus);//添加定时器1 EXPIRE_LightRelay1
        break;
    case 1:
        LightStepStatus = 2;
        GpioRealayC(1, ENABLE);//继电器2吸合
        AddTimer(1, EXPIRE_LightRelay2, TimerScanLight, (void*) &KeyStatus);//添加定时器1 EXPIRE_LightRelay2
        break;
    case 2:
        LightStepStatus = 3;
        GpioRealayC(1, DISABLE);//继电器2断开
        AddTimer(1, EXPIRE_LightRelayF, TimerScanLight, (void*) &KeyStatus);//添加定时器1 EXPIRE_LightRelayF
        break;
    case 3:
        LightStepStatus = 4;
        GpioFanC(ENABLE);//开启风扇
        break;
    case 4:
        LightStepStatus = 5;
        GpioRealayC(0, DISABLE);//继电器1断开
        GpioRealayC(1, DISABLE);//继电器2断开 如果跳过步骤3也要停止继电器2
        AddTimer(1, EXPIRE_LightFan, TimerScanLight, (void*) &KeyStatus);//添加定时器1 EXPIRE_LightFan
        break;
    case 5:
        LightStepStatus = 0;
        GpioFanC(DISABLE);//停止风扇
        break;
    }

    USART_LightStateRead(UART_1);
    USART1_SendDataProce();
    USART_LightStateRead(UART_2);
    USART2_SendDataProce();
}//增加故障后进入5状态

/**
  * @brief  快门周期控制定时器,初始化后周期被定时器自动启动
  * @param
  TimerId 定时器ID
  * @retval none
  */
void ShutTimScan(void)
{
    static uint8_t ShutterTime = 0;
    
    if(ShutterAMStatus)
    {
        if(!ShutterTime)
        {
            ShutterTime = Flash_ShutterTime - 1;//0时触发，所以要减1
            ShutterOnOffStatus = !ShutterOnOffStatus;
            GpioShutterC((FunctionalState)ShutterOnOffStatus);//控制快门
        }
        else
            ShutterTime--;
        ShutTime_Start();
    }
    else
        ShutterTime = 0;
}

/**
  * @brief  快门手动开关控制
  * @param  none
  * @retval none
*/
static void Proc_ShutterAM(void)
{
    if(!ShutterAMStatus)//手动状态时有效
    {
        ShutterOnOffStatus = !ShutterOnOffStatus;
        GpioShutterC((FunctionalState)ShutterOnOffStatus);//控制快门
    }
    KeyStatusLED();
}

/**
  * @brief  快门手自动切换
  * @param  none
  * @retval none
*/
static void Proc_ShutterSW(void)
{
    if(ShutterAMStatus)//自动状态时有效
    {
        ShutterAMStatus = 0;
    }
    else
    {
        ShutterAMStatus = 1;
        ShutTimScan();//ShutTime_Start();
    }
    KeyStatusLED();
    LCD_DisplayShutter(ShutterAMStatus, Flash_ShutterTime);//更新LCD显示
}

/**
  * @brief  快门时间设置状态
  * @param  none
  * @retval none
*/
static void Proc_ShutterTI(void)
{
    KeyStatus = STATE_SHUTTER_TI;//改变状态
    KeyStatusLED();
    VAR_ShutterTime = Flash_ShutterTime;//赋值中间变量
    LCD_DisplayShutter(ShutterAMStatus, VAR_ShutterTime);//更新LCD显示 传值VAR_ShutterTime
    KeyInPutInit();//开启光标光标闪烁
    AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  滤光片位置设置状态
  * @param  none
  * @retval no
*/
static void Proc_FilterLO(void)
{
    KeyStatus = STATE_FILTER_LO;//改变状态
    KeyStatusLED();
    VAR_FilterLoc = Flash_FilterLoc;//赋值中间变量
    LCD_DisplayFilter(VAR_FilterLoc, Flash_FilterDescrip[Flash_FilterInd[Flash_FilterLoc-1]]);//更新LCD显示 传值VAR_FilterLoc
    KeyInPutInit();//开启光标光标闪烁
    AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  滤光片型号设置状态
  * @param  none
  * @retval no
*/
static void Proc_FilterTY(void)
{
    KeyStatus = STATE_FILTER_TY;//改变状态
    KeyStatusLED();
    VAR_FilterInd = Flash_FilterInd[Flash_FilterLoc - 1];//赋值中间变量
    LCD_DisplayFilter(Flash_FilterLoc, Flash_FilterDescrip[VAR_FilterInd]);//更新LCD显示 传值Flash_FilterDescrip[VAR_FilterInd]
    KeyInPutInit();//开启光标光标闪烁
    AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  光强强度设置状态
  * @param  none
  * @retval none
*/
static void Proc_LIGHTLI(void)
{
    KeyStatus = STATE_LIGHT_LI;//改变状态
    KeyStatusLED();
    VAR_LightIntensity = Flash_LightIntensity;//赋值中间变量
    LCD_DisplayLightIn(VAR_LightIntensity);//更新LCD显示 传值VAR_LightIntensity
    KeyInPutInit();//开启光标光标闪烁
    AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  光强强度设置手自动状态
  * @param  none
  * @retval none
*/
static void Proc_LIGHTAU(void)
{
    KeyStatus = STATE_LIGHT_AU;//改变状态
    KeyStatusLED();
    VAR_LightStatus = Flash_LightStatus;//赋值中间变量
    LCD_DisplaySetLightAUStatus(VAR_LightStatus);//更新LCD显示 传值VAR_LightStatus
    KeyInPutInit();//开启光标光标闪烁
    AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  光强强度设置百分化最小值状态
  * @param  none
  * @retval none
*/
static void Proc_LIGHTMin(void)
{
    KeyStatus = STATE_LIGHT_Min;//改变状态
    KeyStatusLED();
    VAR_LightMini = Flash_LightMini;//赋值中间变量
    LCD_DisplaySetLightMinStatus(ADCPercentConvert[1], VAR_LightMini);
    KeyInPutInit();//开启光标光标闪烁
    AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
    AddTimer(5, EXPIRE_LIGHT_ADC, TimerScanLightADCUpdata, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  光强强度设置百分化最大值状态
  * @param  none
  * @retval none
*/
static void Proc_LIGHTMax(void)
{
    KeyStatus = STATE_LIGHT_Max;//改变状态
    KeyStatusLED();
    VAR_LightMaxi = Flash_LightMaxi;//赋值中间变量
    LCD_DisplaySetLightMaxStatus(ADCPercentConvert[1], VAR_LightMaxi);
    KeyInPutInit();//开启光标光标闪烁
    AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
    AddTimer(5, EXPIRE_LIGHT_ADC, TimerScanLightADCUpdata, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  设置设备通讯地址
  * @param  none
  * @retval none
*/
static void Proc_RemotAdd(void)
{
    KeyStatus = STATE_REMOTE_ADD;//改变状态
    KeyStatusLED();
    VAR_RemoteAdd = Flash_RemoteAddr;//赋值中间变量
    LCD_DisplaySetRemoteAddStatus(VAR_RemoteAdd);
    KeyInPutInit();//开启光标光标闪烁
    AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  数据输入右移位
  * @param  none
  * @retval none
*/
static void Proc_InPutSWUP(void)
{
    KeyStatusLED();
    if(KeyInPutStat.KeyInPutCurrent < KeyInPutStat.KeyInPutEnd)
        KeyInPutStat.KeyInPutCurrent++;
    else
        KeyInPutStat.KeyInPutCurrent = KeyInPutStat.KeyInPutStart;
    if(KeyInPutStat.KeyInPutCurrent || KeyInPutStat.KeyInPutLine)//所有数据不从0位显示
        LCD_CursorBlinkOn(KeyInPutStat.KeyInPutLine, KeyInPutStat.KeyInPutCurrent);//开启光标光标闪烁
    AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  数据输入左移位
  * @param  none
  * @retval none
*/
static void Proc_InPutSWDOWN(void)
{
    KeyStatusLED();
    if(KeyInPutStat.KeyInPutCurrent > KeyInPutStat.KeyInPutStart)
        KeyInPutStat.KeyInPutCurrent--;
    else
        KeyInPutStat.KeyInPutCurrent = KeyInPutStat.KeyInPutEnd;
    if(KeyInPutStat.KeyInPutCurrent || KeyInPutStat.KeyInPutLine)//所有数据不从0位显示
        LCD_CursorBlinkOn(KeyInPutStat.KeyInPutLine, KeyInPutStat.KeyInPutCurrent);//开启光标光标闪烁
    AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  灯泡使用时间查看状态
  * @param  none
  * @retval none
*/
static void Proc_LIGHTTI(void)
{
    KeyStatus = STATE_LIGHT_TI;//改变状态
    KeyStatusLED();
    LCD_DisplayLightTi(Flash_LightTimeHour, Flash_LightTimeMinu);//更新LCD显示 传值Flash_LightTimeHour Flash_LightTimeMinu
    AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  远程本地切换函数
  * @param  none
  * @retval none
*/
static void Proc_REMOTESE(void)
{
    KeyStatus = STATE_REMOTE_SE;//改变状态
    KeyStatusLED();
    Flash_RemoteStatus = !Flash_RemoteStatus;
    LCD_DisplayRemote(Flash_RemoteStatus, Flash_RemoteAddr);//更新LCD显示 传值Flash_RemoteStatus
    FlashDataRefresh = 1;//Flash数据需要写入标识 0不需要写入 1要从新写入
    AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  查看故障代码函数
  * @param  none
  * @retval none
*/
static void Proc_ERRORLO(void)
{
    KeyStatus = STATE_ERROR_LO;//改变状态
    KeyStatusLED();
    AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
    AddTimer(4, EXPIRE_ERROR, TimerScanErroUpdata, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  上下按键调整
  * @param  Direction 0自减 1自加
  * @retval none
*/
static void Proc_KEYUPDOWN(uint8_t Direction)
{
    switch(KeyStatus)
    {
	case STATE_IDLE:
		break;
    case STATE_SHUTTER_TI://快门时间设置状态
        if(Direction)
        {
            if((VAR_ShutterTime + (uint32_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent))) <= SHUTTERTIMEMAX)
                VAR_ShutterTime += (uint32_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent));
//            else
//                VAR_ShutterTime = 1;
        }
        else
        {
            if(VAR_ShutterTime > (uint32_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent)))
                VAR_ShutterTime -= (uint32_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent));
//            else
//                VAR_ShutterTime = SHUTTERTIMEMAX;
        }
        LCD_DisplayShutter(ShutterAMStatus, VAR_ShutterTime);//更新LCD显示
        LCD_CursorBlinkOn(KeyInPutStat.KeyInPutLine, KeyInPutStat.KeyInPutCurrent);
        break;
    case STATE_FILTER_LO://滤光片位置设置状态
        if(Direction)
        {
            if(VAR_FilterLoc < FILTERLOCMEMAX)
                VAR_FilterLoc++;
            else
                VAR_FilterLoc = 1;
        }
        else
        {
            if(VAR_FilterLoc > 1)
                VAR_FilterLoc--;
            else
                VAR_FilterLoc = FILTERLOCMEMAX;
        }
        LCD_DisplayFilter(VAR_FilterLoc, Flash_FilterDescrip[Flash_FilterInd[VAR_FilterLoc-1]]);//更新LCD显示
        LCD_CursorBlinkOn(KeyInPutStat.KeyInPutLine, KeyInPutStat.KeyInPutCurrent);
        break;
    case STATE_FILTER_TY://滤光片型号设置状态
        if(Direction)//当前滤光片位置索引表对应的索引号增加 0~FILTERINDMEMAX-1
        {
            if(VAR_FilterInd < Flash_FilterIndNumber-1)
                VAR_FilterInd++;
            else
                VAR_FilterInd = 0;
        }
        else
        {
            if(VAR_FilterInd > 0)
                VAR_FilterInd--;
            else
                VAR_FilterInd = Flash_FilterIndNumber-1;
        }
        LCD_DisplayFilter(Flash_FilterLoc, Flash_FilterDescrip[VAR_FilterInd]);//更新LCD显示
        LCD_CursorBlinkOn(KeyInPutStat.KeyInPutLine, KeyInPutStat.KeyInPutCurrent);
        break;
    case STATE_LIGHT_LI://光强强度设置状态
        if(Direction)
        {
            if((VAR_LightIntensity + (uint16_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent))) <= LIGHTINTMAX)
                VAR_LightIntensity += (uint16_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent));
//            else
//                VAR_LightIntensity = 0;
        }
        else
        {
            if(VAR_LightIntensity >= (uint16_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent)))
                VAR_LightIntensity -= (uint16_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent));
//            else
//                VAR_LightIntensity = LIGHTINTMAX;
        }
        LCD_DisplayLightIn(VAR_LightIntensity);//更新LCD显示
        LCD_CursorBlinkOn(KeyInPutStat.KeyInPutLine, KeyInPutStat.KeyInPutCurrent);
        break;
    case STATE_LIGHT_AU://光强强度控制设置手自动
        VAR_LightStatus = !VAR_LightStatus;
        LCD_DisplaySetLightAUStatus(VAR_LightStatus);//更新LCD显示
        LCD_CursorBlinkOn(KeyInPutStat.KeyInPutLine, KeyInPutStat.KeyInPutCurrent);
        break;
    case STATE_LIGHT_Min://光强强度控制设置最小值
        if(Direction)
        {
            if((VAR_LightMini + (uint32_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent))) <= LIGHTConvertMin)
                VAR_LightMini += (uint32_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent));
//            else
//                VAR_LightMini = 0;
        }
        else
        {
            if(VAR_LightMini >= (uint32_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent)))
                VAR_LightMini -= (uint32_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent));
//            else
//                VAR_LightMini = LIGHTConvertMin;
        }
        LCD_DisplaySetLightMinStatus(ADCPercentConvert[1], VAR_LightMini);//更新LCD显示
        LCD_CursorBlinkOn(KeyInPutStat.KeyInPutLine, KeyInPutStat.KeyInPutCurrent);
        break;
    case STATE_LIGHT_Max://光强强度控制设置最大值
        if(Direction)
        {
            if((VAR_LightMaxi + (uint32_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent))) <= LIGHTConvertMAX)
                VAR_LightMaxi += (uint32_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent));
//            else
//                VAR_LightMaxi = 0;
        }
        else
        {
            if(VAR_LightMaxi >= (uint32_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent)))
                VAR_LightMaxi -= (uint32_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent));
//            else
//                VAR_LightMaxi = LIGHTConvertMAX;
        }
        LCD_DisplaySetLightMaxStatus(ADCPercentConvert[1], VAR_LightMaxi);//更新LCD显示
        LCD_CursorBlinkOn(KeyInPutStat.KeyInPutLine, KeyInPutStat.KeyInPutCurrent);
        break;
    case STATE_REMOTE_ADD://远程地址设置状态
        if(Direction)
        {
            if((VAR_RemoteAdd + (uint16_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent))) <= REMOTEADDRMAX)
                VAR_RemoteAdd += (uint16_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent));
//            else
//                VAR_RemoteAdd = 0;
        }
        else
        {
            if(VAR_RemoteAdd >= (uint16_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent)))
                VAR_RemoteAdd -= (uint16_t)pow(10, (KeyInPutStat.KeyInPutEnd - KeyInPutStat.KeyInPutCurrent));
//            else
//                VAR_RemoteAdd = REMOTEADDRMAX;
        }
        LCD_DisplaySetRemoteAddStatus(VAR_RemoteAdd);//更新LCD显示
        LCD_CursorBlinkOn(KeyInPutStat.KeyInPutLine, KeyInPutStat.KeyInPutCurrent);
        break;
	case STATE_LIGHT_TI:
		break;
	case STATE_REMOTE_SE:
		break;
	case STATE_ERROR_LO:
		break;
    default:
        break;
    }
}

/**
  * @brief  按键下按
  * @param  none
  * @retval none
*/
static void Proc_KEYDOWN(void)
{
    KeyStatusLED();
    Proc_KEYUPDOWN(0);
    AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  按键上按
  * @param  none
  * @retval none
*/
static void Proc_KEYUP(void)
{
    KeyStatusLED();
    Proc_KEYUPDOWN(1);
    AddTimer(0, EXPIRE_ScanStatus, TimerScanStatus, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  按键下按长按
  * @param  none
  * @retval none
*/
static void Proc_KEYDOWNLong(void)
{
    AddTimer(2, EXPIRE_KeyUpDown, TimerScanUpDown, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  按键上按长按
  * @param  none
  * @retval none
*/
static void Proc_KEYUPLong(void)
{
    AddTimer(2, EXPIRE_KeyUpDown, TimerScanUpDown, (void*) &KeyStatus);//增加定时器
}

/**
  * @brief  设置状态确认
  * @param  none
  * @retval none
*/
static void Proc_IDLECOFI(void)
{
    //用中间变量更新原始值
    switch(KeyStatus)
    {
	case STATE_IDLE:
		break;
    case STATE_SHUTTER_TI://快门时间设置状态
        Flash_ShutterTime = VAR_ShutterTime;
        break;
    case STATE_FILTER_LO://滤光片位置设置状态
        Flash_FilterLoc = VAR_FilterLoc;
        break;
    case STATE_FILTER_TY://滤光片型号设置状态
        Flash_FilterInd[Flash_FilterLoc - 1] = VAR_FilterInd;
        break;
    case STATE_LIGHT_LI://光强强度设置状态
        Flash_LightIntensity = VAR_LightIntensity;
        break;
    case STATE_LIGHT_AU://光强强度控制设置手自动
//        Flash_LightStatus = VAR_LightStatus;
//        Flash_LightMini = VAR_LightMini;
//        Flash_LightMaxi = VAR_LightMaxi;
//        Flash_RemoteAddr = VAR_RemoteAdd;
        break;
    case STATE_LIGHT_Min://光强强度控制设置最小值
//        Flash_LightStatus = VAR_LightStatus;
//        Flash_LightMini = VAR_LightMini;
//        Flash_LightMaxi = VAR_LightMaxi;
//        Flash_RemoteAddr = VAR_RemoteAdd;
        break;
    case STATE_LIGHT_Max://光强强度控制设置最大值
//        Flash_LightStatus = VAR_LightStatus;
//        Flash_LightMini = VAR_LightMini;
//        Flash_LightMaxi = VAR_LightMaxi;
//        Flash_RemoteAddr = VAR_RemoteAdd;
        break;
    case STATE_REMOTE_ADD://远程地址设置状态
        Flash_LightStatus = VAR_LightStatus;
        Flash_LightMini = VAR_LightMini;
        Flash_LightMaxi = VAR_LightMaxi;
        Flash_RemoteAddr = VAR_RemoteAdd;
        break;
	case STATE_LIGHT_TI:
		break;
	case STATE_REMOTE_SE:
		break;
	case STATE_ERROR_LO:
		break;
    default:
        break;
    }
    KeyStatus = STATE_IDLE;//改变状态
    KeyStatusLED();
//    if(!StepRUNStatus && !StepErrorStatus && (StepPositionTarget != Flash_FilterLoc))//如果初始化完成步进电机转到指定位置 放在main函数执行
//    {
//        StepPositionTarget = Flash_FilterLoc;
//        Step_RotatSet();
//    }//调整步进电机
    LCD_DisplayIdle();//更新LCD显示
    FlashDataRefresh = 1;//Flash数据需要写入标识 0不需要写入 1要从新写入
//    DelTimer(3);//定时器函数内根据状态判断，STATE_IDLE状态时自动删除定时器
    DelTimer(0);
}

/**
  * @brief  查看状态退出
  * @param  none
  * @retval none
*/
static void Proc_IDLEEXIT(void)
{
    KeyStatus = STATE_IDLE;//改变状态
    KeyStatusLED();
    LCD_DisplayIdle();//更新LCD显示
//    DelTimer(3);//定时器函数内根据状态判断，STATE_IDLE状态时自动删除定时器
    DelTimer(0);
}

//获取状态机函数
pfKeyScanProcess KeyStatusArray[STATE_COUNT][EVENT_COUNT] = 
{
    {Proc_ShutterAM, Proc_ShutterSW,  Proc_FilterLO,   Proc_LIGHTLI,  NULL,          NULL,          Proc_LIGHTAU, Proc_ShutterTI, Proc_FilterTY, Proc_LIGHTTI, Proc_REMOTESE,    Proc_ERRORLO,   Proc_IDLEEXIT},//STATE_IDLE
    {NULL,           Proc_IDLECOFI,   Proc_InPutSWDOWN,Proc_InPutSWUP,Proc_KEYDOWN,  Proc_KEYUP,    NULL,         NULL,           NULL,          NULL,         Proc_KEYDOWNLong, Proc_KEYUPLong, Proc_IDLEEXIT},//STATE_SHUTTER_TI
    {NULL,           NULL,            Proc_IDLECOFI,   NULL,          Proc_KEYDOWN,  Proc_KEYUP,    NULL,         NULL,           NULL,          NULL,         Proc_KEYDOWNLong, Proc_KEYUPLong, Proc_IDLEEXIT},//STATE_FILTER_LO
    {NULL,           NULL,            Proc_IDLECOFI,   NULL,          Proc_KEYDOWN,  Proc_KEYUP,    NULL,         NULL,           NULL,          NULL,         Proc_KEYDOWNLong, Proc_KEYUPLong, Proc_IDLEEXIT},//STATE_FILTER_TY
    {NULL,           Proc_InPutSWDOWN,Proc_InPutSWUP,  Proc_IDLECOFI, Proc_KEYDOWN,  Proc_KEYUP,    NULL,         NULL,           NULL,          NULL,         Proc_KEYDOWNLong, Proc_KEYUPLong, Proc_IDLEEXIT},//STATE_LIGHT_LI
    {Proc_LIGHTMin,  NULL,            NULL,            NULL,          Proc_KEYDOWN,  Proc_KEYUP,    NULL,         NULL,           NULL,          NULL,         Proc_KEYDOWNLong, Proc_KEYUPLong, Proc_IDLEEXIT},//STATE_LIGHT_AU
    {Proc_LIGHTMax,  Proc_InPutSWDOWN,Proc_InPutSWUP,  NULL,          Proc_KEYDOWN,  Proc_KEYUP,    NULL,         NULL,           NULL,          NULL,         Proc_KEYDOWNLong, Proc_KEYUPLong, Proc_IDLEEXIT},//STATE_LIGHT_Min
    {Proc_RemotAdd,  Proc_InPutSWDOWN,Proc_InPutSWUP,  NULL,          Proc_KEYDOWN,  Proc_KEYUP,    NULL,         NULL,           NULL,          NULL,         Proc_KEYDOWNLong, Proc_KEYUPLong, Proc_IDLEEXIT},//STATE_LIGHT_Max
    {Proc_IDLECOFI,  Proc_InPutSWDOWN,Proc_InPutSWUP,  NULL,          Proc_KEYDOWN,  Proc_KEYUP,    NULL,         NULL,           NULL,          NULL,         Proc_KEYDOWNLong, Proc_KEYUPLong, Proc_IDLEEXIT},//STATE_REMOTE_ADD
    {NULL,           NULL,            NULL,            Proc_IDLEEXIT, NULL,          NULL,          NULL,         NULL,           NULL,          NULL,         NULL,             NULL,           Proc_IDLEEXIT},
    {NULL,           NULL,            NULL,            NULL,          Proc_IDLEEXIT, NULL,          NULL,         NULL,           NULL,          NULL,         NULL,             NULL,           Proc_IDLEEXIT},    
    {NULL,           NULL,            NULL,            NULL,          NULL,          Proc_IDLEEXIT, NULL,         NULL,           NULL,          NULL,         NULL,             NULL,           Proc_IDLEEXIT}
};

/**
  * @brief  按键按下抬起状态扫描函数
  * @param  KeyScanPro
  * @retval no
*/
void KeyScanUpDowStatus(KeyScanPro_TypeDef *KeyScanPro)
{
    for(uint8_t i=0; i<MAX_KEY_COUNT; i++)
    {
        if(Flash_RemoteStatus && i != 4)//按键5远程就地切换按键
            continue;
        if(!GPIO_ReadInputDataBit(GPIOPortKEY[i].GPIOx, GPIOPortKEY[i].GPIO_Pin))//低电平按键按下
        {
            if(KeyScanPro[i].KeyPressCnt < 0xFF00)
                KeyScanPro[i].KeyPressCnt++;
            if((KeyScanPro[i].KeyPressCnt > KEY_SHORT_PRESS_CNT && KeyScanPro[i].KeyPressCnt < KEY_LONG_PRESS_CNT) && KeyStatusArray[KeyStatus][i])
                GpioLEDC(KEYNUMBERSTAR + i, ENABLE);//处理按键指示灯,按键指示灯从4开始至9
            else if(KeyScanPro[i].KeyPressCnt > KEY_LONG_PRESS_CNT && KeyStatusArray[KeyStatus][MAX_KEY_COUNT + i])
                GpioLEDC(KEYNUMBERSTAR + i, ENABLE);//处理按键指示灯,按键指示灯从4开始至9

//            if(KeyScanPro[i].KeyPressCnt > KEY_SUPER_LONG_PRESS_CNT && KeyScanPro[i].KeySuperLongEventFlg == 0)
//            {
//                AddEventToQueue(&KeyEvenQueue, (KeyEvent_TypeDef)(2 * MAX_KEY_COUNT + i));
//            }
            
            if(KeyScanPro[i].KeyPressCnt > KEY_LONG_PRESS_CNT && KeyScanPro[i].KeyLongEventFlg == 0)
            {
                KeyScanPro[i].KeyLongEventFlg = 1;
                AddEventToQueue(&KeyEvenQueue, (KeyEvent_TypeDef)(MAX_KEY_COUNT + i));
            }
        }
        else//无按键按下
        {
            if(KeyScanPro[i].KeyPressCnt > KEY_SHORT_PRESS_CNT && KeyScanPro[i].KeyPressCnt < KEY_LONG_PRESS_CNT)
            {
                //按键放开处理LED放置在状态函数中
//                KeyScanPro[i].KeyShortEventFlg = 1;
                AddEventToQueue(&KeyEvenQueue, (KeyEvent_TypeDef)(i));
            }
//            else if(KeyScanPro[i].KeyPressCnt > KEY_LONG_PRESS_CNT && KeyScanPro[i].KeyPressCnt < KEY_SUPER_LONG_PRESS_CNT)
//            {
//                KeyScanPro[i].KeyLongEventFlg = 1;
//                AddEventToQueue(&KeyEvenQueue, (KeyEvent_TypeDef)(MAX_KEY_COUNT + i));
//            }
            //清除按键计数 标识
            memset(&KeyScanPro[i],0,sizeof(KeyScanPro_TypeDef));
        }
    }
}

/**
  * @brief  按键状态机扫描函数
  * @param
  KeyEvenQueue 消息队列结构体指针
  KeyStatus  状态机当前状态
  * @retval no
  */
void KeyScanStatusPro(KeyEventQueue_TypeDef *KeyEvenQueue)
{
    KeyEvent_TypeDef KeyEvent;
    pfKeyScanProcess  pfKeyEventPro = NULL;//函数指针

    if(ReadEventFromQueue(KeyEvenQueue, &KeyEvent))
    {
        if((pfKeyEventPro = (pfKeyScanProcess)(KeyStatusArray[KeyStatus][KeyEvent])))//函数指针非空执行
            pfKeyEventPro();
    }
}

/**
  * @brief  点灯按键状态扫描函数
  * @param  KeyScanPro
  * @retval no
*/
void KeyScanLightStatus(void)
{
    static uint8_t OnCount = 0;
    static uint8_t OffCount = 0;
    static uint8_t KeyStatus = 0;//1标识上一次已经按下未抬起
    
    if(!GPIO_ReadInputDataBit(GPIOPortKEY[MAX_KEY_COUNT].GPIOx, GPIOPortKEY[MAX_KEY_COUNT].GPIO_Pin))//低电平按键按下
    {
        OffCount = 0;
        if(OnCount > 4)//执行三次判断
        {
            if(!KeyStatus)
            {
                if(LightStepStatus == 0)// || LightStepStatus == 5
                {
                    LightStepStatus = 0;//启动
                    KeyLightStep();
                }
                else if(LightStepStatus < 5)
                {
                    LightStepStatus = 4;//停止
                    KeyLightStep();
                }
                KeyStatus = 1;
            }
        }
        else
            OnCount++;
    }
    else//无按键按下
    {
        OnCount = 0;
        if(OffCount > 4)//执行三次判断
        {
            KeyStatus = 0;
        }
        else
            OffCount++;
    }
}

/**
  * @brief  灯使用时间计时函数
  * @param  KeyScanPro
  * @retval no
*/
void LightTimeSum(void)
{
    static uint8_t LightTimeTemp = 0;
    static uint8_t LightLastStep = 0;
    
    if(LightStepStatus == 4)
    {
        LightTimeTemp++;
        if(LightTimeTemp >= LIGHTTIMESTEP)
        {
            LightTimeTemp = 0;
            if(Flash_LightTimeMinu < LIGHTMINUMAX && Flash_LightTimeHour < LIGHTHOURMAX)
            {
                Flash_LightTimeMinu += LIGHTTIMESTEP;
                if(Flash_LightTimeMinu > LIGHTMINUMAX)
                {
                    Flash_LightTimeHour += Flash_LightTimeMinu/60;
                    if(Flash_LightTimeHour < LIGHTHOURMAX)
                    {
                        Flash_LightTimeMinu %= 60;
                        FlashDataRefresh = 1;
                    }
                }
                else
                    FlashDataRefresh = 1;
            }
        }
    }
    else if(LightStepStatus == 5 && LightLastStep == 4)
    {
        if(LightTimeTemp && Flash_LightTimeMinu < LIGHTMINUMAX && Flash_LightTimeHour < LIGHTHOURMAX)
        {
            Flash_LightTimeMinu += LightTimeTemp;
            if(Flash_LightTimeMinu > LIGHTMINUMAX)
            {
                Flash_LightTimeHour += Flash_LightTimeMinu/60;
                if(Flash_LightTimeHour < LIGHTHOURMAX)
                {
                    Flash_LightTimeMinu %= 60;
                    FlashDataRefresh = 1;
                }
            }
            else
                FlashDataRefresh = 1;
        }
        LightTimeTemp = 0;
    }
    LightLastStep = LightStepStatus;
}

/**
  * @brief  点灯顺控错误检测函数
  * @param  KeyScanPro
  * @retval no
*/
void LightScanErrStatus(void)
{
    if(RealayErrStatus[0] || RealayErrStatus[1] || FanErrStatus || DocErrStatus || TemErrStatus)
    {
        LightErrStatus = 1;
        if(LightStepStatus > 0 && LightStepStatus < 5)
        {
            LightStepStatus = 4;
            KeyLightStep();
        }
    }
    else
        LightErrStatus = 0;
}

/**
  * @brief  系统扫描故障状态
  * @param  KeyScanPro
  * @retval no
*/
void SystemScanStatus(void)
{
    uint8_t TemError = 0;

    if (StepErrorStatus)
        TemError |= 0x01;
    if (ShutErrStatus)
        TemError |= 0x02;
    if (RealayErrStatus[0] || RealayErrStatus[1])
        TemError |= 0x04;
    if (FanErrStatus)
        TemError |= 0x08;
    if (DocErrStatus)
        TemError |= 0x10;
    if (TemErrStatus)
        TemError |= 0x20;
    SystemErrorStatus = TemError;
}

/**
  * @brief  系统LED指示灯
  * @param  KeyScanPro
  * @retval no
*/
void SystemLedStatus(void)
{
    if(LightStepStatus > 0 && LightStepStatus < 5)
        GpioLEDC(10, ENABLE);//灯泡运行
    else
        GpioLEDC(10, DISABLE);//灯泡停止

    if(ShutErrStatus)
        GpioLEDC(2, DISABLE);//快门
    else if(ShutRunStatus < 2)
        GpioLEDC(2, (FunctionalState)ShutRunStatus);//快门

    GpioLEDC(1, (FunctionalState)Flash_RemoteStatus);//远程就地

    if(SystemErrorStatus)
        GpioLEDC(0, ENABLE);//故障
    else
        GpioLEDC(0, DISABLE);//无故障
}

/**
  * @brief  Light强度控制函数
  * @param  no
  * @retval no
*/
void LUXContr(void)
{
    static uint16_t DACOUT = 0;
    
    if(Flash_LightStatus)
    {
        if(ADCLUXPercent[1] < (Flash_LightIntensity - 1))
        {
            if(DACOUT > 0)
                DACOUT--;
        }
        else if(ADCLUXPercent[1] > (Flash_LightIntensity + 1))
        {
            if(DACOUT < 4095)
                DACOUT++;
        }
    }
    else
    {
        DACOUT = (100-Flash_LightIntensity) * 4095 / 100;//输出反比例0V最亮
    }
    DACMDA_OUT(DACOUT);//点灯成功后 位式控制DAC数据输出0-10V
}