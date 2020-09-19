#ifndef __STMFLASH_H
#define __STMFLASH_H

#ifdef __cplusplus
extern "C"
{
#endif

//#define STM32_FLASH_SIZE        512             //所选STM32的FLASH容量大小(单位为K)  //大容量
#define STM32_FLASH_SIZE 128 //所选STM32的FLASH容量大小(单位为K)  //中等容量

#if STM32_FLASH_SIZE < 256
#define STM_SECTOR_SIZE 1024 //每页字节数
#else
#define STM_SECTOR_SIZE 2048 //每页字节数
#endif

#define STM32_FLASH_BASE 0x08000000 //STM32 FLASH的起始地址0x08000000
#ifdef STM32F10X_LD                 /* STM32F10X_LD */
#define STM32_FLASH_STAT 0x08003C00 //操作起始地址 0x08007C00 32K;  0x08003C00 16K;
#define STM32_FLASH_STAT 0x08003FFE //操作终止地址 0x08007FFE 32K;  0x08003FFE 16K;
#endif
#ifdef STM32F10X_MD                 /* STM32F10X_MD */
#define STM32_FLASH_STAT 0x0800FC00 //操作起始地址 0x0801FC00 128K; 0x0800FC00 64K
#define STM32_FLASH_END 0x0800FFFE  //操作终止地址 0x0801FFFE 128K; 0x0800FFFE 64K
#endif
#ifdef STM32F10X_HD /* STM32F10X_HD */ //范围是2K
#define STM32_FLASH_STAT 0x0803F800    //操作起始地址 0x0807F800 512K; 0x0803F800 265K
#define STM32_FLASH_END 0x0803FFFF     //操作终止地址 0x0807F800 512K; 0x0803FFFF 256K
#endif

#define Flash_DataNumber 60 //读取存储Flash数据长度 前面存储14个，索引8个，索引总是1个，字符9*4合计72个(固定21个不读写，可新增9个)
#define REMOTEADDRMAX 255   //设备远程通讯地址最大值
#define FILTERLOCMEMAX 8    //滤光片最大位置
#define LIGHTINTMAX 100     //光照强度值最大100
#define LIGHTConvertMin 99999999
#define LIGHTConvertMAX 99999999
#define LIGHTHOURMAX 99999   //灯泡使用时间小时显示5位
#define LIGHTMINUMAX 59      //灯泡使用时间分钟显示2位
#define SHUTTERTIMEMAX 99999 //快门时间显示5位 单位s
#define FILTERINDMEMAX 30    //滤光片文字描述符最大值
#define FILTERINDMEDEF 27    //滤光片文字描述符 默认内存存储27个

  extern uint8_t FlashDataRefresh;                   //Flash数据需要写入标识 0不需要写入 1要从新写入
  extern uint16_t Flash_DataArray[Flash_DataNumber]; //数据存储数组

  extern uint16_t Flash_RemoteAddr;                   //设备远程通讯地址
  extern uint16_t Flash_RemoteStatus;                 //设备远程就地状态 0就地控制 1远程控制
  extern uint32_t Flash_LightTimeHour;                //显示使用时间小时4字节
  extern uint16_t Flash_LightTimeMinu;                //显示使用时间分钟2字节
  extern uint16_t Flash_LightIntensity;               //灯光强度设定值 2个字节
  extern uint16_t Flash_LightStatus;                  //光强度控制状态 0手动控制输出 1比例控制输出
  extern uint32_t Flash_LightMini;                    //光强度测量值百分化最小值
  extern uint32_t Flash_LightMaxi;                    //光强度测量值百分化最大值
  extern uint32_t Flash_ShutterTime;                  //快门时间间隔4字节，单位s
  extern uint16_t Flash_FilterLoc;                    //滤光片位置2个字节
  extern uint16_t Flash_FilterInd[FILTERLOCMEMAX];    //滤光片轮8个标识字符索引，每个2字节，共16字节，即8个16位
  extern uint16_t Flash_FilterIndNumber;              //2个字节1个16位记录总数 索引号从0开始
  extern char Flash_FilterDescrip[FILTERINDMEMAX][8]; //滤光片标识选择列表，每个8字节，长度n

  uint16_t STMFLASH_ReadHalfWord(uint32_t faddr); //读出半字
  uint32_t STMFLASH_ReadWord(uint32_t faddr);     //读出字
  void STMFLASH_Init(void);
  void STMFLASH_ReadHalfBuffer(uint32_t ReadAddr, uint16_t *pBuffer, uint16_t NumToRead);    //从指定地址开始读出指定长度的数据
  void STMFLASH_ReadWordBuffer(uint32_t ReadAddr, uint32_t *pBuffer, uint16_t NumToRead);    //从指定地址开始读出指定长度的数据
  void STMFLASH_WriteHalfBuffer(uint32_t WriteAddr, uint16_t *pBuffer, uint16_t NumToWrite); //从指定地址开始写入指定长度的数据
  void STMFLASH_WriteWordBuffer(uint32_t WriteAddr, uint32_t *pBuffer, uint16_t NumToWrite);
  void FLASH_DataRead(void);
  void FLASH_DataWrite(void);

#ifdef __cplusplus
}
#endif

#endif