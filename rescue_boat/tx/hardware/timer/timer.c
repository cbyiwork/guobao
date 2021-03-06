#include "timer.h"

void TIM2_Configuration(void)
{
	
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	/* TIM3 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	
	/* ---------------------------------------------------------------
	TIM3CLK 即PCLK1=36MHz
	TIM3CLK = 36 MHz, Prescaler = 7200, TIM3 counter clock = 5K,即改变一次为5K,周期就为10K
	--------------------------------------------------------------- */
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 0xffff; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =(7200-1); //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	/* Enables the Update event for TIM3 */
	//TIM_UpdateDisableConfig(TIM3,ENABLE); 	//使能 TIM3 更新事件 
	
	/* TIM IT enable */
	
	/* TIM3 enable counter */
	TIM_Cmd(TIM2, ENABLE);  //使能TIMx外设
	
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
	TIM3CLK 录麓PCLK1=36MHz
	TIM3 Configuration: generate 1 PWM signals :
    TIM3CLK = 36 MHz, Prescaler = 0x0, TIM3 counter clock = 36 MHz
    TIM3 ARR Register = 900 => TIM3 Frequency = TIM3 counter clock/(ARR + 1)
    TIM3 Frequency = 36 KHz.
    TIM3 Channel2 duty cycle = (TIM3_CCR2/ TIM3_ARR)* 100 
	TIM3CLK = 36 MHz, Prescaler = 0, TIM3 counter clock = 36MHz
	--------------------------------------------------------------- */
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = period-1; //脡猫脰脙脭脷脧脗脪禄赂枚赂眉脨脗脢脗录镁脳掳脠毛禄卯露炉碌脛脳脭露炉脰脴脳掳脭脴录脛麓忙脝梅脰脺脝脷碌脛脰碌	 80K
	TIM_TimeBaseStructure.TIM_Prescaler =72-1; //脡猫脰脙脫脙脌麓脳梅脦陋TIMx脢卤脰脫脝碌脗脢鲁媒脢媒碌脛脭陇路脰脝碌脰碌  虏禄路脰脝碌
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //脡猫脰脙脢卤脰脫路脰赂卯:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM脧貌脡脧录脝脢媒脛拢脢陆
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //赂霉戮脻TIM_TimeBaseInitStruct脰脨脰赂露篓碌脛虏脦脢媒鲁玫脢录禄炉TIMx碌脛脢卤录盲禄霉脢媒碌楼脦禄
	
	/* Output Compare Active Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //脩隆脭帽露篓脢卤脝梅脛拢脢陆:TIM脗枚鲁氓驴铆露脠碌梅脰脝脛拢脢陆2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //卤脠陆脧脢盲鲁枚脢鹿脛脺
	TIM_OCInitStructure.TIM_Pulse = 0; //脡猫脰脙麓媒脳掳脠毛虏露禄帽卤脠陆脧录脛麓忙脝梅碌脛脗枚鲁氓脰碌
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //脢盲鲁枚录芦脨脭:TIM脢盲鲁枚卤脠陆脧录芦脨脭赂脽
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);  //赂霉戮脻TIM_OCInitStruct脰脨脰赂露篓碌脛虏脦脢媒鲁玫脢录禄炉脥芒脡猫TIMx
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //脢鹿脛脺TIMx脭脷CCR2脡脧碌脛脭陇脳掳脭脴录脛麓忙脝梅

	TIM_OC2Init(TIM3,&TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);


	TIM_ARRPreloadConfig(TIM3, ENABLE); //脢鹿脛脺TIMx脭脷ARR脡脧碌脛脭陇脳掳脭脴录脛麓忙脝梅
	
	/* TIM3 enable counter */
	TIM_Cmd(TIM3, ENABLE);  //脢鹿脛脺TIMx脥芒脡猫
	
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


