#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
extern "C"
{
#endif

//定义时间
#define KEY_SHORT_PRESS_CNT 2        //短按键时间 10ms倍数
#define KEY_SHORT_LONG_PRESS_CNT 100 //短按键时间 10ms倍数
#define KEY_LONG_PRESS_CNT 300       //长按键时间 10ms倍数
#define KEY_SUPER_LONG_PRESS_CNT 800 //超长按键时间 10ms倍数

#define EXPIRE_ScanStatus 6000 //状态机超时 1min
#define EXPIRE_ScanShutter 100 //周期1s去判断快门状态，到指定时间后反转 快门软件定时器
#define EXPIRE_LightRelay1 400 //点灯程序第一个继电器吸合后延时启动第二个继电器 4s
#define EXPIRE_LightRelay2 200 //点灯程序第二个继电器吸合延时断开 1.5s
#define EXPIRE_LightRelayF 150 //点灯程序第二个继电器断开延时启动风扇 1.5s
#define EXPIRE_LightFan 48000  //点灯成功后断电风扇延时 8min
#define EXPIRE_KeyUpDown 10    //设置状态下长按上下键快速增减参数
#define EXPIRE_LCDB 50         //设置状态下LCD闪烁频率
#define EXPIRE_ERROR 20        //故障显示时刷新频率频率
#define EXPIRE_LIGHT_ADC 100    //灯ADC测量值显示时刷新频率频率

#define MAX_KEY_EVENT_CNT 5 //事件列队最多允许的数量
#define MAX_KEY_COUNT 6     //最大按键数量
// #define MAX_STATE_COUNT 12  //最大按键状态数量
// #define MAX_EVENT_COUNT 13  //最大按键事件数量

#define KEYNUMBERSTAR 4  //按键指示灯起始数值
#define KEYNUMBERSTOP 10 //按键指示灯结束数值

#define LIGHTTIMESTEP 20 //定义多少分钟写入一次Flash

#define KEYINPUTLINE_SHUTTER 1 //光标闪烁所在的行
#define KEYINPUTLINE_FILTERLO 0
#define KEYINPUTLINE_FILTERTY 0
#define KEYINPUTLINE_LIGHT 0
#define KEYINPUTLINE_LIGHTAU 1
#define KEYINPUTLINE_LIGHTMIN 1
#define KEYINPUTLINE_LIGHTMAX 1
#define KEYINPUTLINE_REMOTEADD 0

#define KEYINPUTSTART_SHUTTER 6 //光标闪烁开始位置
#define KEYINPUTSTART_FILTERLO 1
#define KEYINPUTSTART_FILTERTY 8
#define KEYINPUTSTART_LIGHT 16
#define KEYINPUTSTART_LIGHTAU 0
#define KEYINPUTSTART_LIGHTMIN 12
#define KEYINPUTSTART_LIGHTMAX 12
#define KEYINPUTSTART_REMOTEADD 17

#define KEYINPUTEND_SHUTTER 10
#define KEYINPUTEND_FILTERLO 1
#define KEYINPUTEND_FILTERTY 8
#define KEYINPUTEND_LIGHT 18
#define KEYINPUTEND_LIGHTAU 0
#define KEYINPUTEND_LIGHTMIN 19
#define KEYINPUTEND_LIGHTMAX 19
#define KEYINPUTEND_REMOTEADD 19

    //定义状态
    typedef enum
    {
        STATE_IDLE,       //空闲装置
        STATE_SHUTTER_TI, //快门时间设置状态
        STATE_FILTER_LO,  //滤光片位置设置状态
        STATE_FILTER_TY,  //滤光片型号设置状态
        STATE_LIGHT_LI,   //光强强度设置状态
        STATE_LIGHT_AU,   //光强强度控制设置手自动
        STATE_LIGHT_Min,  //光强强度控制设置最小值
        STATE_LIGHT_Max,  //光强强度控制设置最大值
        STATE_REMOTE_ADD, //远程地址设置状态
        STATE_LIGHT_TI,   //灯泡使用时间查看状态
        STATE_REMOTE_SE,  //远程就地控制设置状态
        STATE_ERROR_LO,    //故障查看状态

        STATE_COUNT
    } KeyStatus_TypeDef;

    //定义事件
    typedef enum
    {
        KEY1_SHORT_PRESS_EVENT, //K1设置按键短按事件
        KEY2_SHORT_PRESS_EVENT, //K2设置按键短按事件
        KEY3_SHORT_PRESS_EVENT, //K3设置按键短按事件
        KEY4_SHORT_PRESS_EVENT, //K4设置按键短按事件
        KEY5_SHORT_PRESS_EVENT, //K5设置按键短按事件
        KEY6_SHORT_PRESS_EVENT, //K6设置按键短按事件
        KEY1_LONG_PRESS_EVENT,  //K1设置按键长按事件
        KEY2_LONG_PRESS_EVENT,  //K2设置按键长按事件
        KEY3_LONG_PRESS_EVENT,  //K3设置按键长按事件
        KEY4_LONG_PRESS_EVENT,  //K4设置按键长按事件
        KEY5_LONG_PRESS_EVENT,  //K5设置按键长按事件
        KEY6_LONG_PRESS_EVENT,  //K6设置按键长按事件
        KEY_TIME_EVENT,          //设置超时按事件

        EVENT_COUNT
    } KeyEvent_TypeDef;

    typedef struct
    {
        uint8_t KeyInPutCurrent; //当前位置，计算时用KeyInPutCurrent-KeyInPutStart，范围是KeyInPutStart-KeyInPutEnd
        uint8_t KeyInPutStart;
        uint8_t KeyInPutEnd;
        uint8_t KeyInPutLine; //行
    } KeyInPut_TypeDef;       //按照设置数据时记录当前输入位置结构体

    typedef struct
    {
        //    uint8_t KeyShortEventFlg;//按键抬起操作，所以不用标志位
        uint8_t KeyShortLongEventFlg;
        uint8_t KeyLongEventFlg;
        uint8_t KeySuperLongEventFlg;
        uint8_t KeyDelayCnt;
        uint16_t KeyPressCnt; //按键闭合计时器,检测到闭合一次加1
    } KeyScanPro_TypeDef;     //按键扫描信息结构体  扫描按键状态时记录信息

    typedef struct
    {
        uint8_t WritePos; //指向下一次被写入的位置
        uint8_t ReadPos;  //指向下一次被读取的位置
        KeyEvent_TypeDef KeyEventQueue[MAX_KEY_EVENT_CNT];
    } KeyEventQueue_TypeDef; //按键事件列队结构体

    extern KeyEventQueue_TypeDef KeyEvenQueue; //按键事件列队
    extern KeyStatus_TypeDef KeyStatus;        //按键状态机
    extern KeyScanPro_TypeDef KeyScanPro[MAX_KEY_COUNT];

    extern uint8_t SystemErrorStatus;  //标记系统运行状态，0位步进电机故障 1位快门故障 2位继电器故障 3位风扇故障 4位机箱门故障 5位温度故障
    extern uint8_t LightStepStatus;    //标记点灯程序是否正在进行，大于0进行
    extern uint8_t LightErrStatus;     //标记点灯关键设备故障，0正常 1故障
    extern uint8_t ShutterAMStatus;    //快门手动自动状态 0手动 1自动
    extern uint8_t ShutterOnOffStatus; //快门开关状态 0快门关 1快门开

    void KeyScanUpDowStatus(KeyScanPro_TypeDef *KeyScanPro);
    void KeyScanStatusPro(KeyEventQueue_TypeDef *KeyEvenQueue);

    void ShutTimScan(void);

    void LCD_DisplayIdle(void);
    void KeyLightStep(void);
    void KeyScanLightStatus(void);
    void LightTimeSum(void);
    void LightScanErrStatus(void);
    void SystemScanStatus(void);
    void SystemLedStatus(void);
    void LUXContr(void);

#ifdef __cplusplus
}
#endif

#endif