#include <math.h>
//#include <stdlib.h>
#include "stm32f0xx.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_adc.h"
#include "stm32f0xx_dac.h"
#include "stm32f0xx_dma.h"
#include "gpio.h"
#include "adcdma.h"

//uint16_t ADCVref = 0;//内部参考电压的ADC值
//uint16_t ADCVDD = 0;//VDD电压值 单位mV

uint8_t ADCConvertStatus = 0; //标记ADC转换是否正在进行 0未执行转换或者未完成 1转换完成
uint32_t ADC_offset;          //ADC1 ADC3偏移

uint8_t ADCErrorStatus[ADCChannelConver] = {0}; //0无故障，1是低报警，2高报警
static uint16_t ADCBuffer[ADCChannelCount][ADCChannelConver] = {0};                            //ADC1 DMA传输后的数组 ADCChannelCount行，ADCChannelConver列
static uint8_t ADCDataCount[ADCChannelConver] = {0};                                           //记录已经测量多少数据 大于等于ADCChannelCircul不再增长
static uint8_t ADCDataPoint[ADCChannelConver] = {0};                                           //计数，标识当前剔除数据的位置
static float ADCDataSum[ADCChannelConver] = {0};                                               //记录所有数据累加和
static float ADCData[ADCChannelCircul][ADCChannelConver] = {0};                                //ADC模拟量数据存储
static uint8_t ADCMultiCount[ADCChannelConver] = {0};                                          //放大倍数后要等待几个周期在取有限制
static uint8_t ADCMultiNumber[ADCChannelConver] = {0};                                         //放大倍数，查询ADCMultArry得到实际放大倍数
static float ADCMultArry[ADCMULTNUMBER] = {ADCMULTONE, ADCMULTTWO, ADCMULTTHREE, ADCMULTFOUR}; //ADC放大倍数数组
static float ADCLUXConvert[ADCChannelConver] = {0};                                            //将测量光照度转换为电流
uint8_t ADCLUXPercent[ADCChannelConver] = {0};                                                 //为了控制计算转换为百分数
uint32_t ADCPercentConvert[ADCChannelConver] = {0};                                            //存储ADC浮点数转换为32位整数

/**
  * @brief  ADCDMA中断初始化
  * @param  None
  * @retval None
  */
static void ADCDMA_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    SYSCFG_DMAChannelRemapConfig(SYSCFG_DMARemap_ADC1, DISABLE); //Default DMA channel is mapped to the selected peripheral
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;     //ADC1
    NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  ADCOVR中断初始化
  * @param  None
  * @retval None
  */
// static void ADCOVR_NVIC_Config(void)
// {
//     NVIC_InitTypeDef NVIC_InitStructure;

//     NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn; //ADC1
//     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
//     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//     NVIC_Init(&NVIC_InitStructure);
// }

/**
  * @brief  ADCDMA初始化
  * @param  None
  * @retval None
  */
void ADCMDA_Init(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    //ADC1
    DMA_DeInit(DMA1_Channel1);                                                  //复位DMA1,将外设DMA1的全部寄存器重设为缺省值
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;             //DMA外设ADC基地址(u32)&ADC1->DR
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADCBuffer;                 //DMA内存基地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                          //数据传输方向，从ADC读取发送到内存
    DMA_InitStructure.DMA_BufferSize = ADCChannelCount * ADCChannelConver;      //DMA通道的DMA缓存的大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            //外设地址寄存器不变
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     //内存地址寄存器递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //数据宽度为16位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         //数据宽度为16位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                             //工作在循环缓存模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                       //DMA通道拥中优先级
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                //DMA通道没有设置为内存到内存传输
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);                                //根据以上参数初始化DMA1
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);                             //Transfer complete interrupt mask
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TE, ENABLE);                             //Transfer error interrupt mask
    DMA_Cmd(DMA1_Channel1, ENABLE);

    ADCDMA_NVIC_Config();

    ADC_Cmd(ADC1, DISABLE);                                             //失能ADC1
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                  //ADC1和ADC2工作在独立模式
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;                        //模数转换工作在扫描模式（多通道）
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                  //模数转换工作在连续模式
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //转换由软件而不是外部触发启动
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;              //ADC数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = ADCChannelConver;              //规定了顺序进行规则转换的ADC通道的数目
    ADC_Init(ADC1, &ADC_InitStructure);

    //    ADC_TempSensorVrefintCmd(ENABLE);//非常重要，如果不开启是不能测量温度和内部参考电压

    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_239Cycles5); //ADC1-5 ADC_SampleTime_239Cycles5 ADC9M频率 采样频率是9M/(239.5+12.5)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 2, ADC_SampleTime_239Cycles5); //ADC1-6 ADC_SampleTime_239Cycles5
    //    ADC_RegularChannelConfig(ADC1,ADC_Channel_17,3,ADC_SampleTime_239Cycles5);   //ADC1-17 ADC_SampleTime_239Cycles5

    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE); //使能ADC1

    ADC_ResetCalibration(ADC1); //使能复位校准  不影响DMA里的数据
    while (ADC_GetResetCalibrationStatus(ADC1))
        ;                       //等待复位校准结束
    ADC_StartCalibration(ADC1); //开启AD校准
    while (ADC_GetCalibrationStatus(ADC1))
        ;                                      //等待校准结束
    ADC_offset = ADC_GetConversionValue(ADC1); //ADC1校准后的偏置 ADC1->DR

    //ADC_SoftwareStartConvCmd(ADC3, ENABLE);//软件启动转换
}

/**
  * @brief  DAC初始化
  * @param  None
  * @retval None
  */
void DACMDA_Init(void)
{
    DAC_InitTypeDef DAC_InitStructure;

    DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;                         //不使用外部触发 TEN1=0
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;           //不产生方波
    DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0; //屏蔽/幅值选择器，这个变量只在使用波形发生器的时候才有用
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;            //输出缓存 BOFF1=1
    DAC_Init(DAC_Channel_1, &DAC_InitStructure);                              //选择通道1

    //DACRatio = 1;//DAC输出比例，1~20
    //DACOut = DACRatio*DACGrads;//DAC输出值0~2047
    //DAC_SetChannel1Data(DAC_Align_12b_R, DACOut);                                    //12位右对齐方式
    DAC_Cmd(DAC_Channel_1, ENABLE); //
}

/**
  * @brief  DAC初始化
  * @param  None
  * @retval DACOutData 0~4095
  */
void DACMDA_OUT(uint16_t DACOutData)
{
    DAC_SetChannel1Data(DAC_Align_12b_R, DACOutData);
}

/**
  * @brief  ADC数据转换
  * @param  None
  * @retval None
  */
void ADCDMA_DataProce(void)
{
    uint8_t i, j, k;
    uint8_t TempMulti = 0;
    volatile uint32_t ADCTemp = 0;    //存储ADC通道转换后的ADC平均值
    volatile float ADCFConvert = 0.0; //存储光电二极管电流

    for (i = 0; i < ADCChannelConver; i++)
    {
        if(ADCMultiCount[i] > 0)//改变放大倍数后 一定周期后测试数据有效
        {
            ADCMultiCount[i]--;
            continue;
        }

        for (j = 0; j < ADCChannelCount - 1; j++) //ADC采样后从小到大排序 可用快速法排序
        {
            for (k = j + 1; k < ADCChannelCount; k++)
            {
                if (ADCBuffer[j][i] > ADCBuffer[k][i])
                {
                    ADCTemp = ADCBuffer[j][i];
                    ADCBuffer[j][i] = ADCBuffer[k][i];
                    ADCBuffer[k][i] = ADCTemp;
                }
            }
        }
        ADCTemp = 0;
        for (j = 0; j < ADCChannelCount / 2; j++) //ADC采样后丢弃一半 累加求和
        {
            ADCTemp += ADCBuffer[ADCChannelCount / 4 + j][i];
        }
        ADCTemp /= (ADCChannelCount / 2);
        ADCFConvert = (float)ADCTemp * 3 / 2 * 3300 / 4096 / ADCMultArry[ADCMultiNumber[i]]; //计算绝对值
        if (ADCFConvert > 9.5)                                                               //偏置电压9.5mv
            ADCFConvert = 152.93 + (ADCFConvert - 9.5) * 15.5397;//单位是100uw/cm2
        else
            ADCFConvert = 0;

        //缓冲累加平均
        ADCDataSum[i] += ADCFConvert;
        ADCDataSum[i] -= ADCData[ADCDataPoint[i]][i];
        ADCData[ADCDataPoint[i]][i] = ADCFConvert;
        ADCDataPoint[i]++;
        ADCDataPoint[i] %= ADCChannelCircul;
        if (ADCDataCount[i] < ADCChannelCircul)
            ADCDataCount[i]++;
        if(ADCDataSum[i] < 0)
            ADCLUXConvert[i] = 0;
        else
            ADCLUXConvert[i] = ADCDataSum[i] / ADCDataCount[i];

        //转换为0~100 如果超出范围 减最小值 除以最大值减最小值
        ADCPercentConvert[i] = (uint32_t)ADCLUXConvert[i];
        if (ADCPercentConvert[i] <= Flash_LightMini)
            ADCLUXPercent[i] = 0;
        else if (ADCPercentConvert[i] >= Flash_LightMaxi)
            ADCLUXPercent[i] = 100;
        else if (Flash_LightMaxi > Flash_LightMini)
        {
            ADCLUXPercent[i] = (uint8_t)((ADCPercentConvert[i] - Flash_LightMini) * 100 / (Flash_LightMaxi - Flash_LightMini));
            if (ADCLUXPercent[i] > 100)
                ADCLUXPercent[i] = 100;
        }

        // 改变放大倍数
        TempMulti = ADCMultiNumber[i];
        if (ADCTemp > 3276) //适当切换放大倍数 3700 改为20%
        {
            if (ADCMultiNumber[i] > 0)
                ADCMultiNumber[i]--;
            else
                ADCMultiNumber[i] = 0;
        }
        else if (ADCTemp < 819)//300 改为20%
        {
            if (ADCMultiNumber[i] < (ADCMULTNUMBER - 1))
                ADCMultiNumber[i]++;
            else
                ADCMultiNumber[i] = ADCMULTNUMBER - 1;
        }
        if (ADCMultiNumber[i] != TempMulti)
        {
            ADCMultiCount[i] = ADCMULTCOUNT;
            GpioADCRatio(i, ADCMultiNumber[i]);
        }
    }
}