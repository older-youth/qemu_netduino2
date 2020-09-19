#include <string.h>
#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "stmflash.h"


/*不建议使用片内Flash，需要根据系统时钟设置等待周期，导致中断响应延迟*/
static uint16_t STMFLASH_HALFBUF[STM_SECTOR_SIZE/2];//最多是2K字节
static uint32_t STMFLASH_WORDBUF[STM_SECTOR_SIZE/4];//最多是1K字

uint8_t FlashDataRefresh = 0;//Flash数据需要写入标识 0不需要写入 1要从新写入
uint16_t Flash_DataArray[Flash_DataNumber] = {0};//数据存储数组
uint16_t Flash_RemoteAddr = 0;//设备远程通讯地址
uint16_t Flash_RemoteStatus = 0;//设备远程就地状态 0就地控制 1远程控制
uint32_t Flash_LightTimeHour = 0;//显示使用时间小时4字节
uint16_t Flash_LightTimeMinu = 0;//显示使用时间分钟2字节
uint16_t Flash_LightIntensity = 0;//灯光强度设定值 2个字节
uint16_t Flash_LightStatus = 0;//光强度控制状态 0手动控制输出 1比例控制输出
uint32_t Flash_LightMini = 0;//光强度测量值百分化最小值
uint32_t Flash_LightMaxi = 0;//光强度测量值百分化最大值
uint32_t Flash_ShutterTime = 0;//快门时间间隔4字节，单位s
uint16_t Flash_FilterLoc = 0;//滤光片位置2个字节
uint16_t Flash_FilterInd[FILTERLOCMEMAX] = {0};//滤光片轮8个标识字符索引，每个2字节，共16字节，即8个16位
uint16_t Flash_FilterIndNumber = 0;//2个字节1个16位记录总数 索引号从0开始，计数从1开始
//滤光片标识选择列表，每个8字节，长度n
char Flash_FilterDescrip[FILTERINDMEMAX][8] = {\
    " FB350 "," FB360 "," FB365 "," FB380 "," FB420 "," FB430 "," FB445 "," FB450 "," FB470 "," FB490 ",\
    " FB500 "," FB520 "," FB550 "," FB560 "," FB570 "," FB590 "," FB600 "," FB620 "," FB635 "," FB640 ",\
    " FB650 "," FB670 "," FB700 "," FB750 "," FB800 "," FB850 "," FB900 "};
/*
char Flash_FilterDescrip[FILTERINDMEMAX][8] = {" VIS420"," VIS435"," VIS450"," VIS475"," VIS500"," VIS520"," VIS550",\
    " VIS575"," VIS600"," VIS620"," VIS650"," VIS675"," VIS700","  UV350","  UV365","  UV380"};
*/


/**
  * @brief  FLASH_SetLatency 初始化
  * @param none
  * @retval none
  */
void STMFLASH_Init(void)
{
    FLASH_SetLatency(FLASH_Latency_2);
}

/**
  * @brief  读取指定地址的半字(16位数据)
  * @param
    faddr:读地址(此地址必须为2的倍数!!)
  * @retval 读取数据
  */
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr)
{
    return *(uint16_t*)faddr; 
}


/**
  * @brief  从指定地址开始读出指定长度的数据
  * @param
    ReadAddr:起始地址
    pBuffer:存储数据指针
    NumToRead:读取数据长度
  * @retval 读取数据长度
  */
void STMFLASH_ReadHalfBuffer(uint32_t ReadAddr, uint16_t *pBuffer, uint16_t NumToRead)
{
    for(uint16_t i=0; i<NumToRead; i++)
    {
        *pBuffer = STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节
        pBuffer++;
        ReadAddr+=2;//偏移2个字节
    }
}


/**
  * @brief  读取指定地址的字(32位数据)
  * @param
    faddr:读地址(此地址必须为2的倍数!!)
  * @retval 读取数据
  */
uint32_t STMFLASH_ReadWord(uint32_t faddr)
{
    return *(uint32_t*)faddr; 
}


/**
  * @brief  从指定地址开始读出指定长度的数据
  * @param
    ReadAddr:起始地址
    pBuffer:存储数据指针
    NumToRead:读取数据长度
  * @retval 读取数据长度
  */
void STMFLASH_ReadWordBuffer(uint32_t ReadAddr, uint32_t *pBuffer, uint16_t NumToRead)
{
    for(uint16_t i=0; i<NumToRead; i++)
    {
        *pBuffer = STMFLASH_ReadWord(ReadAddr);//读取4个字节
        pBuffer++;
        ReadAddr+=4;//偏移4个字节
    }
}


/**
  * @brief  从指定地址开始写指定长度的数据
  * @param
WriteAddr:起始地址
pBuffer:写入数据指针
  * @retval 写入数据长度
  */
void STMFLASH_WriteHalf_NoCheck(uint32_t WriteAddr, uint16_t *pBuffer, uint16_t NumToWrite)
{
    for(uint16_t i=0; i<NumToWrite; i++)
    {
        FLASH_ProgramHalfWord(WriteAddr, *pBuffer);
        pBuffer++;
        WriteAddr+=2;//地址增加2.
    }
}


/**
  * @brief  从指定地址开始写指定长度的数据
  * @param
WriteAddr:起始地址
pBuffer:写入数据指针
  * @retval 写入数据长度
  */
void STMFLASH_WriteHalfBuffer(uint32_t WriteAddr, uint16_t *pBuffer, uint16_t NumToWrite)
{
    uint32_t FLASH_offaddr;             //去掉0X08000000后的地址
    uint32_t FLASH_secpos;	        //扇区地址
    uint16_t FLASH_secoff;	        //扇区内偏移地址(16位字计算)
    uint16_t FLASH_secsur;              //扇区内剩余地址(16位字计算)
    uint16_t i;
    
    if(WriteAddr<STM32_FLASH_STAT ||WriteAddr>=STM32_FLASH_END)
        return;//非法地址
    FLASH_Unlock();			//解锁
    FLASH_offaddr=WriteAddr-STM32_FLASH_BASE;           //去掉0X08000000后的地址
    FLASH_secpos=FLASH_offaddr/STM_SECTOR_SIZE;         //扇区地址  0~127 for STM32F103RBT6 0~255 for STM32F103ZET6
    FLASH_secoff=(FLASH_offaddr%STM_SECTOR_SIZE)/2;	//在扇区内的偏移(2个字节为基本单位.)
    FLASH_secsur=STM_SECTOR_SIZE/2-FLASH_secoff;	//扇区剩余空间大小
    if(NumToWrite<=FLASH_secsur)
        FLASH_secsur=NumToWrite;//不大于该扇区范围
    while(1) 
    {
        STMFLASH_ReadHalfBuffer(FLASH_secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE, STMFLASH_HALFBUF, STM_SECTOR_SIZE/2);//读出整个扇区的内容
        for(i=0;i<FLASH_secsur;i++)//校验读出的数据是否全为0xFFFF
        {
            if(STMFLASH_HALFBUF[FLASH_secoff+i]!=0xFFFF)
                break;//需要擦除  	  
        }
        if(i<FLASH_secsur)//需要擦除 把Flash的单元从"1"写为"0"时，无需执行擦除操作即可进行连续写操作。把Flash的单元从"0"写为"1"时，则需要执行Flash擦除操作。
        {
            FLASH_ErasePage(FLASH_secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//擦除这个扇区
            for(i=0;i<FLASH_secsur;i++)//复制
            {
                STMFLASH_HALFBUF[i+FLASH_secoff]=pBuffer[i];	  
            }
            STMFLASH_WriteHalf_NoCheck(FLASH_secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_HALFBUF,STM_SECTOR_SIZE/2);//写入整个扇区  
        }
        else
            STMFLASH_WriteHalf_NoCheck(WriteAddr, pBuffer, FLASH_secsur);//写已经擦除了的,直接写入扇区剩余区间. 				   
        if(NumToWrite==FLASH_secsur)
            break;//写入结束了,写入数据量小于当前页字节数量
        else//写入未结束
        {
            FLASH_secpos++;             //扇区地址增1
            FLASH_secoff=0;             //偏移位置为0
            pBuffer+=FLASH_secsur;  	//指针偏移
            WriteAddr+=FLASH_secsur;	//写地址偏移
            NumToWrite-=FLASH_secsur;	//字节(16位)数递减
            if(NumToWrite>(STM_SECTOR_SIZE/2))
                FLASH_secsur=STM_SECTOR_SIZE/2;//下一个扇区还是写不完
            else FLASH_secsur=NumToWrite;//下一个扇区可以写完了
        }
    };
    FLASH_Lock();//上锁
}


/**
  * @brief  从指定地址开始写指定长度的数据
  * @param
WriteAddr:起始地址
pBuffer:写入数据指针
  * @retval 写入数据长度
  */
void STMFLASH_WriteWord_NoCheck(uint32_t WriteAddr, uint32_t *pBuffer, uint16_t NumToWrite)
{
    for(uint16_t i=0; i<NumToWrite; i++)
    {
        FLASH_ProgramWord(WriteAddr, *pBuffer);
        pBuffer++;
        WriteAddr+=4;//地址增加2.
    }
}


/**
  * @brief  从指定地址开始写指定长度的数据
  * @param
WriteAddr:起始地址
pBuffer:写入数据指针
  * @retval 写入数据长度
  */
void STMFLASH_WriteWordBuffer(uint32_t WriteAddr, uint32_t *pBuffer, uint16_t NumToWrite)
{
    uint32_t FLASH_offaddr;             //去掉0X08000000后的地址
    uint32_t FLASH_secpos;	        //扇区地址
    uint16_t FLASH_secoff;	        //扇区内偏移地址(16位字计算)
    uint16_t FLASH_secsur;              //扇区内剩余地址(16位字计算)
    uint16_t i;
    
    if(WriteAddr<STM32_FLASH_STAT ||WriteAddr>=STM32_FLASH_END)
        return;//非法地址
    FLASH_Unlock();			//解锁
    FLASH_offaddr=WriteAddr-STM32_FLASH_BASE;           //去掉0X08000000后的地址
    FLASH_secpos=FLASH_offaddr/STM_SECTOR_SIZE;         //扇区地址  0~127 for STM32F103RBT6 0~255 for STM32F103ZET6
    FLASH_secoff=(FLASH_offaddr%STM_SECTOR_SIZE)/4;	//在扇区内的偏移(2个字节为基本单位.)
    FLASH_secsur=STM_SECTOR_SIZE/4-FLASH_secoff;	//扇区剩余空间大小
    if(NumToWrite<=FLASH_secsur)
        FLASH_secsur=NumToWrite;//不大于该扇区范围
    while(1) 
    {	
        STMFLASH_ReadWordBuffer(FLASH_secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE, STMFLASH_WORDBUF, STM_SECTOR_SIZE/4);//读出整个扇区的内容
        for(i=0;i<FLASH_secsur;i++)//校验读出的数据是否全为0xFFFF
        {
            if(STMFLASH_WORDBUF[FLASH_secoff+i]!=0xFFFFFFFF)
                break;//需要擦除  	  
        }
        if(i<FLASH_secsur)//需要擦除 把Flash的单元从"1"写为"0"时，无需执行擦除操作即可进行连续写操作。把Flash的单元从"0"写为"1"时，则需要执行Flash擦除操作。
        {
            FLASH_ErasePage(FLASH_secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//擦除这个扇区
            for(i=0;i<FLASH_secsur;i++)//复制
            {
                STMFLASH_WORDBUF[i+FLASH_secoff]=pBuffer[i];	  
            }
            STMFLASH_WriteWord_NoCheck(FLASH_secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE, STMFLASH_WORDBUF, STM_SECTOR_SIZE/4);//写入整个扇区  
        }
        else
            STMFLASH_WriteWord_NoCheck(WriteAddr, pBuffer, FLASH_secsur);//写已经擦除了的,直接写入扇区剩余区间. 				   
        if(NumToWrite==FLASH_secsur)
            break;//写入结束了,写入数据量小于当前页字节数量
        else//写入未结束
        {
            FLASH_secpos++;             //扇区地址增1
            FLASH_secoff=0;             //偏移位置为0
            pBuffer+=FLASH_secsur;  	//指针偏移
            WriteAddr+=FLASH_secsur;	//写地址偏移
            NumToWrite-=FLASH_secsur;	//字(32位)数递减
            if(NumToWrite>(STM_SECTOR_SIZE/4))
                FLASH_secsur=STM_SECTOR_SIZE/4;//下一个扇区还是写不完
            else FLASH_secsur=NumToWrite;//下一个扇区可以写完了
        }
    };
    FLASH_Lock();//上锁
}


/**
  * @brief  Flash数据读取初始化
  * @param  no
  * @retval no
  */
void FLASH_DataRead(void)
{
    STMFLASH_ReadHalfBuffer(STM32_FLASH_STAT, Flash_DataArray, Flash_DataNumber);
    
    Flash_RemoteAddr = Flash_DataArray[0];
    if(Flash_RemoteAddr > REMOTEADDRMAX)
        Flash_RemoteAddr = 0;//设备远程通讯地址
    
    Flash_RemoteStatus = Flash_DataArray[1];
    if(Flash_RemoteStatus > 1)
        Flash_RemoteStatus = 0;//0就地控制 1远程控制
    
    //(uint32_t)Flash_DataArray[1] << 16 + Flash_DataArray[2];
    Flash_LightTimeHour = *(uint32_t*)(&Flash_DataArray[2]);
    if(Flash_LightTimeHour > LIGHTHOURMAX)
        Flash_LightTimeHour = 0;//显示使用时间小时4字节
    
    Flash_LightTimeMinu = Flash_DataArray[4];
    if(Flash_LightTimeMinu > LIGHTMINUMAX)
        Flash_LightTimeMinu = 0;//显示使用时间分钟2字节
    
    Flash_LightIntensity = Flash_DataArray[5];
    if(Flash_LightIntensity > LIGHTINTMAX)
        Flash_LightIntensity = 0;//灯光强度设定值 2个字节

    Flash_LightStatus = Flash_DataArray[6];
    if(Flash_LightStatus > 1)
        Flash_LightStatus = 0;//0手动控制输出 1比例控制输出
    
    Flash_LightMini = *(uint32_t*)(&Flash_DataArray[7]);
    if(Flash_LightMini == 0 || Flash_LightMini > LIGHTConvertMin)
        Flash_LightMini = 700;//光强度测量值百分化最小值 180000 350000

    Flash_LightMaxi = *(uint32_t*)(&Flash_DataArray[9]);
    if(Flash_LightMaxi == 0 || Flash_LightMaxi > LIGHTConvertMAX)
        Flash_LightMaxi = 1088;//光强度测量值百分化最大值 372000 800000
    
    Flash_ShutterTime = *(uint32_t*)(&Flash_DataArray[11]);
    if(Flash_ShutterTime < 1 || Flash_ShutterTime > SHUTTERTIMEMAX)
        Flash_ShutterTime = 1;//快门时间间隔4字节

    //增加设定值 最大最小测量值百分化

    Flash_FilterLoc = Flash_DataArray[13];
    if(Flash_FilterLoc < 1 || Flash_FilterLoc > FILTERLOCMEMAX)
        Flash_FilterLoc = 1;//滤光片位置2个字节
    
    for(uint8_t i=0; i<FILTERLOCMEMAX; i++)
    {
        Flash_FilterInd[i] = Flash_DataArray[14 + i];
        if(Flash_FilterInd[i] > FILTERINDMEMAX - 1)
            Flash_FilterInd[i] = 0;
    }
    
    //滤光片字符标识
    Flash_FilterIndNumber = Flash_DataArray[22];
    if(Flash_FilterIndNumber < FILTERINDMEDEF || Flash_FilterIndNumber > FILTERINDMEMAX)
    {
        Flash_FilterIndNumber = FILTERINDMEDEF;//滤光片字符描述记录总数
        if(Flash_FilterIndNumber > FILTERINDMEDEF)
        {
            //字符串拷贝 FILTERINDMEMAX-FILTERINDMEDEF
            for(uint8_t i=0; i<FILTERINDMEMAX-FILTERINDMEDEF; i++)
            {
                Flash_FilterDescrip[FILTERINDMEDEF + i][0] = (uint8_t)(Flash_DataArray[23 + i * 4] >> 8);
                Flash_FilterDescrip[FILTERINDMEDEF + i][1] = (uint8_t)Flash_DataArray[23 + i * 4];
                Flash_FilterDescrip[FILTERINDMEDEF + i][2] = (uint8_t)(Flash_DataArray[24 + i * 4] >> 8);
                Flash_FilterDescrip[FILTERINDMEDEF + i][3] = (uint8_t)Flash_DataArray[24 + i * 4];
                Flash_FilterDescrip[FILTERINDMEDEF + i][4] = (uint8_t)(Flash_DataArray[25 + i * 4] >> 8);
                Flash_FilterDescrip[FILTERINDMEDEF + i][5] = (uint8_t)Flash_DataArray[25 + i * 4];
                Flash_FilterDescrip[FILTERINDMEDEF + i][6] = (uint8_t)(Flash_DataArray[26 + i * 4] >> 8);
                Flash_FilterDescrip[FILTERINDMEDEF + i][7] = (uint8_t)Flash_DataArray[26 + i * 4];
            }
        }
    }
}


/**
  * @brief  Flash数据写入，保存掉电数据
  * @param  no
  * @retval no
  */
void FLASH_DataWrite(void)
{
    Flash_DataArray[0] = Flash_RemoteAddr;//设备远程通讯地址
    
    Flash_DataArray[1] = Flash_RemoteStatus;//0就地控制 1远程控制
    
    *(uint32_t*)(&Flash_DataArray[2]) = Flash_LightTimeHour;
    
    Flash_DataArray[4] = Flash_LightTimeMinu;
    
    Flash_DataArray[5] = Flash_LightIntensity;//灯光强度设定值 2个字节

    Flash_DataArray[6] = Flash_LightStatus;//0手动控制输出 1比例控制输出
    
    *(uint32_t*)(&Flash_DataArray[7]) = Flash_LightMini;//光强度测量值百分化最小值

    *(uint32_t*)(&Flash_DataArray[9]) = Flash_LightMaxi;//光强度测量值百分化最大值

    *(uint32_t*)(&Flash_DataArray[11]) = Flash_ShutterTime;

    Flash_DataArray[13] = Flash_FilterLoc;//滤光片位置2个字节
    
    for(uint8_t i=0; i<FILTERLOCMEMAX; i++)
    {
        Flash_DataArray[14 + i] = Flash_FilterInd[i];
    }
    
    Flash_DataArray[22] = Flash_FilterIndNumber;//滤光片字符标识
    if(Flash_FilterIndNumber > FILTERINDMEDEF)
    {
        //字符串拷贝 FILTERINDMEMAX-FILTERINDMEDEF
        for(uint8_t i=0; i<FILTERINDMEMAX-FILTERINDMEDEF; i++)
        {
            Flash_DataArray[23 + i * 4] = (uint16_t)Flash_FilterDescrip[FILTERINDMEDEF + i][0] << 8 | (uint16_t)Flash_FilterDescrip[FILTERINDMEDEF + i][1];
            Flash_DataArray[24 + i * 4] = (uint16_t)Flash_FilterDescrip[FILTERINDMEDEF + i][2] << 8 | (uint16_t)Flash_FilterDescrip[FILTERINDMEDEF + i][3];
            Flash_DataArray[25 + i * 4] = (uint16_t)Flash_FilterDescrip[FILTERINDMEDEF + i][4] << 8 | (uint16_t)Flash_FilterDescrip[FILTERINDMEDEF + i][5];
            Flash_DataArray[26 + i * 4] = (uint16_t)Flash_FilterDescrip[FILTERINDMEDEF + i][6] << 8 | (uint16_t)Flash_FilterDescrip[FILTERINDMEDEF + i][7];
        }
    }
    
    STMFLASH_WriteHalfBuffer(STM32_FLASH_STAT, Flash_DataArray, Flash_DataNumber);
}