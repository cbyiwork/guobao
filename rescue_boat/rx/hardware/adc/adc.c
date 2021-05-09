#include "adc.h"
#include "../uart/uart.h"
#include <string.h>
#include "../nrf24l01/nrf24l01.h"
#include "../key/key.h"

#define ADC_Count  6 //每通道采6次
#define ADC_CHS    4 // 4通道

uint16_t AD_Value[ADC_Count][ADC_CHS];
uint16_t adcResult[ADC_CHS];

static void ADC_Config(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
    
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); 
    
    ADC_DeInit(ADC1);  
    
    ADC_InitStructure.ADC_Mode                  = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode          = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode    = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv      = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign             = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel          = ADC_CHS;
    ADC_Init(ADC1, &ADC_InitStructure);                             
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5 );
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_239Cycles5 );
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_239Cycles5 );
    ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_239Cycles5 );
    
    
    ADC_DMACmd(ADC1, ENABLE);   
    ADC_Cmd(ADC1, ENABLE);  
    ADC_ResetCalibration(ADC1);    
    while(ADC_GetResetCalibrationStatus(ADC1));  
    ADC_StartCalibration(ADC1);  
    while(ADC_GetCalibrationStatus(ADC1)); 
}

static void ADC_DMA_Config(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr    = (u32)&(ADC1->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr        = (u32)&AD_Value;
    DMA_InitStructure.DMA_DIR                   = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize            = ADC_Count*ADC_CHS;
    DMA_InitStructure.DMA_PeripheralInc         = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc             = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize    = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize        = DMA_MemoryDataSize_HalfWord; 
    DMA_InitStructure.DMA_Mode                  = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority              = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M                   = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);     
}

static void ADC_gpio_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1,ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);    

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);    

    CTL_STICK_ON();

}

void AdcInit() {
	
	ADC_gpio_Config();
	ADC_Config();
	ADC_DMA_Config();

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    	DMA_Cmd(DMA1_Channel1, ENABLE);  

}


static void getAdcConvertResult(uint16_t result[]){

	int i,j = 0;
	uint16_t val = 0;

	for (i=0;i<ADC_CHS;i++) {
		//dprint("ch[%d]: ", i);
		val = 0;
		for(j=0;j<ADC_Count;j++) {
			val += AD_Value[j][i];
		}
		result[i] = val/ADC_Count;
		//dprint("ch[%d]: %d, ",i,adcResult[i]);

	}
	//dprint("\r\n");

}

void checkCtlStick(){

	int dir = 0;
	int pow = 0;
	uint8_t sendBuf[32] = {0};
	int index = 0;

	getAdcConvertResult(adcResult);

	// left and right
	if (adcResult[2] > adcResult[3]) {
		dir |= 0x8000;
		pow = (adcResult[2] - adcResult[3]) / ADC_STEP;
		dir |= (pow<<8);
	}else {
		dir &= ~0x8000;
		pow = (adcResult[3] - adcResult[2]) / ADC_STEP;
		dir |= (pow<<8);
	}

	if (adcResult[0] > adcResult[1]) {
		dir |= 0x0080;
		pow = (adcResult[0]-adcResult[1]) / ADC_STEP;
		dir |= pow;
	} else {
		dir &= ~0x0080;
		pow = (adcResult[1]-adcResult[0]) / ADC_STEP;
		dir |= pow;
	}

	dprint("ctl stick: %x\r\n", dir);

	memcpy(sendBuf,"gas:",sizeof("gas:")-1);
	index += sizeof("gas:")-1;
	memcpy(sendBuf+index,(u8*)&dir,4);
	index += 4;

	if (g_srMode){
		NRF24L01_TxPacket(sendBuf);
	}
	
	

}





