#ifndef __ADCDMA_H
#define __ADCDMA_H

#ifdef __cplusplus
extern "C"
{
#endif

#define ADCChannelConver 2   //ADC转换通道数 2个光照度
#define ADCChannelCount 48   //每个ADC通道单次DMA转换的次数 被4整除
#define ADCChannelCircul 50//100 //250 //每个ADC通道循环存储的个数 不能大于255,标识位用的uint8类型
#define ADCMULTNUMBER 4      //每个ADC通道放大倍数数量
#define ADCMULTCOUNT 2       //改变放大倍数后 5个周期后测试数据有效

#define ADCMULTONE 4.08    //第一个放大倍数大小，相对值ADC转换值直接除以放大倍数，绝对值要乘以3.3V/2*3/4096/第一个运放的电阻值(1K),得到电流值
#define ADCMULTTWO 35.17   //第一个放大倍数大小，相对值ADC转换值直接除以放大倍数，绝对值要乘以3.3V/2*3/4096/第一个运放的电阻值(1K),得到电流值
#define ADCMULTTHREE 96.7 //第一个放大倍数大小，相对值ADC转换值直接除以放大倍数，绝对值要乘以3.3V/2*3/4096/第一个运放的电阻值(1K),得到电流值
#define ADCMULTFOUR 824.5 //第一个放大倍数大小，相对值ADC转换值直接除以放大倍数，绝对值要乘以3.3V/2*3/4096/第一个运放的电阻值(1K),得到电流值

    extern uint8_t ADCConvertStatus; //标记ADC转换正在进行
    extern uint32_t ADC_offset;      //ADC1 ADC3偏移

    extern uint8_t ADCErrorStatus[ADCChannelConver]; //0无故障，1是低报警，2高报警
    extern uint8_t ADCLUXPercent[ADCChannelConver];      //为了控制计算转换为百分数
    extern uint32_t ADCPercentConvert[ADCChannelConver]; //存储ADC浮点数转换为32位整数

    void ADCMDA_Init(void);
    void ADCDMA_DataProce(void);
    void DACMDA_Init(void);
    void DACMDA_OUT(uint16_t DACOutData);

#ifdef __cplusplus
}
#endif

#endif