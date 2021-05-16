#include "timer.h"

void TIM2_Configuration(void)
{
	
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	/* TIM3 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	
	/* ---------------------------------------------------------------
	TIM3CLK ��PCLK1=36MHz
	TIM3CLK = 36 MHz, Prescaler = 7200, TIM3 counter clock = 5K,���ı�һ��Ϊ5K,���ھ�Ϊ10K
	--------------------------------------------------------------- */
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 0xffff; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms
	TIM_TimeBaseStructure.TIM_Prescaler =(7200-1); //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	
	/* Enables the Update event for TIM3 */
	//TIM_UpdateDisableConfig(TIM3,ENABLE); 	//ʹ�� TIM3 �����¼� 
	
	/* TIM IT enable */
	
	/* TIM3 enable counter */
	TIM_Cmd(TIM2, ENABLE);  //ʹ��TIMx����
	
}

void TIM3_Configuration(uint32_t period)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	GPIO_InitTypeDef gpio_InitStructure;

	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);

	//GPIO_PinRemapConfig(GPIO_FullRemap_TIM2,ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	gpio_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	gpio_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA,&gpio_InitStructure);	

	//GPIO_PinRemapConfig(GPIO_FullRemap_TIM3,ENABLE);	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	/* ---------------------------------------------------------------
	TIM3CLK ¼´PCLK1=36MHz
	TIM3 Configuration: generate 1 PWM signals :
    TIM3CLK = 36 MHz, Prescaler = 0x0, TIM3 counter clock = 36 MHz
    TIM3 ARR Register = 900 => TIM3 Frequency = TIM3 counter clock/(ARR + 1)
    TIM3 Frequency = 36 KHz.
    TIM3 Channel2 duty cycle = (TIM3_CCR2/ TIM3_ARR)* 100 
	TIM3CLK = 36 MHz, Prescaler = 0, TIM3 counter clock = 36MHz
	--------------------------------------------------------------- */
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = period-1; //ÉèÖÃÔÚÏÂÒ»¸ö¸üÐÂÊÂ¼þ×°Èë»î¶¯µÄ×Ô¶¯ÖØ×°ÔØ¼Ä´æÆ÷ÖÜÆÚµÄÖµ	 80K
	TIM_TimeBaseStructure.TIM_Prescaler =72-1; //ÉèÖÃÓÃÀ´×÷ÎªTIMxÊ±ÖÓÆµÂÊ³ýÊýµÄÔ¤·ÖÆµÖµ  ²»·ÖÆµ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //ÉèÖÃÊ±ÖÓ·Ö¸î:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIMÏòÉÏ¼ÆÊýÄ£Ê½
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //¸ù¾ÝTIM_TimeBaseInitStructÖÐÖ¸¶¨µÄ²ÎÊý³õÊ¼»¯TIMxµÄÊ±¼ä»ùÊýµ¥Î»
	
	/* Output Compare Active Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //Ñ¡Ôñ¶¨Ê±Æ÷Ä£Ê½:TIMÂö³å¿í¶Èµ÷ÖÆÄ£Ê½2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //±È½ÏÊä³öÊ¹ÄÜ
	TIM_OCInitStructure.TIM_Pulse = 0; //ÉèÖÃ´ý×°Èë²¶»ñ±È½Ï¼Ä´æÆ÷µÄÂö³åÖµ
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //Êä³ö¼«ÐÔ:TIMÊä³ö±È½Ï¼«ÐÔ¸ß
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);  //¸ù¾ÝTIM_OCInitStructÖÐÖ¸¶¨µÄ²ÎÊý³õÊ¼»¯ÍâÉèTIMx
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //Ê¹ÄÜTIMxÔÚCCR2ÉÏµÄÔ¤×°ÔØ¼Ä´æÆ÷

	TIM_OC2Init(TIM3,&TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);


	TIM_ARRPreloadConfig(TIM3, ENABLE); //Ê¹ÄÜTIMxÔÚARRÉÏµÄÔ¤×°ÔØ¼Ä´æÆ÷
	
	/* TIM3 enable counter */
	TIM_Cmd(TIM3, ENABLE);  //Ê¹ÄÜTIMxÍâÉè
	
}


u32 getTickMs(u32* tmr) {
	u32 interval = 0;
	//interval = SysTick->VAL - *tmr;
	//interval =  *tmr - SysTick->VAL;
	interval = TIM_GetCounter(TIM2) - *tmr;
	//*tmr = TIM_GetCounter(TIM3);
	return (interval/10);

}

void refreshTimer(u32* tmr) {
	*tmr = TIM_GetCounter(TIM2);
}


