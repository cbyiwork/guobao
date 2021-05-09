#include "pwm.h"

void TIM2_Configuration(uint32_t period)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	GPIO_InitTypeDef gpio_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);

	// 实际测试发现，AFIO的使能需要放在GPIO Remap的前面，
	// 否则没有PWM 输出
	//GPIO_PinRemapConfig(GPIO_FullRemap_TIM2,ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	gpio_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
	gpio_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB,&gpio_InitStructure);	

	GPIO_PinRemapConfig(GPIO_FullRemap_TIM2,ENABLE);	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	//不分频。PWM频率=72000000/900=80Khz
	/* ---------------------------------------------------------------
	TIM3CLK 即PCLK1=36MHz
	TIM3 Configuration: generate 1 PWM signals :
    TIM3CLK = 36 MHz, Prescaler = 0x0, TIM3 counter clock = 36 MHz
    TIM3 ARR Register = 900 => TIM3 Frequency = TIM3 counter clock/(ARR + 1)
    TIM3 Frequency = 36 KHz.
    TIM3 Channel2 duty cycle = (TIM3_CCR2/ TIM3_ARR)* 100 
	TIM3CLK = 36 MHz, Prescaler = 0, TIM3 counter clock = 36MHz
	--------------------------------------------------------------- */
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = period-1; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 80K
	TIM_TimeBaseStructure.TIM_Prescaler =72-1; //设置用来作为TIMx时钟频率除数的预分频值  不分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	/* Output Compare Active Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_Pulse = 0; //设置待装入捕获比较寄存器的脉冲值
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性高
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx
	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);  //使能TIMx在CCR2上的预装载寄存器

	TIM_OC4Init(TIM2,&TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM2,TIM_OCPreload_Enable);


	TIM_ARRPreloadConfig(TIM2, ENABLE); //使能TIMx在ARR上的预装载寄存器
	
	/* TIM3 enable counter */
	TIM_Cmd(TIM2, ENABLE);  //使能TIMx外设
}

void TIM1_Configuration(uint32_t period)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	GPIO_InitTypeDef gpio_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO,ENABLE);
	gpio_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	gpio_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB,&gpio_InitStructure);	
	
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	
	//不分频。PWM频率=72000000/900=80Khz
	/* ---------------------------------------------------------------
	TIM3CLK 即PCLK1=36MHz
	TIM3 Configuration: generate 1 PWM signals :
    TIM3CLK = 36 MHz, Prescaler = 0x0, TIM3 counter clock = 36 MHz
    TIM3 ARR Register = 900 => TIM3 Frequency = TIM3 counter clock/(ARR + 1)
    TIM3 Frequency = 36 KHz.
    TIM3 Channel2 duty cycle = (TIM3_CCR2/ TIM3_ARR)* 100 
	TIM3CLK = 36 MHz, Prescaler = 0, TIM3 counter clock = 36MHz
	--------------------------------------------------------------- */
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = period-1; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 80K
	TIM_TimeBaseStructure.TIM_Prescaler =36-1; //设置用来作为TIMx时钟频率除数的预分频值  不分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	/* Output Compare Active Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_Pulse = 0; //设置待装入捕获比较寄存器的脉冲值
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIMx在CCR2上的预装载寄存器

	TIM_OC2Init(TIM1, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx
	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIMx在CCR2上的预装载寄存器

	TIM_OC3Init(TIM1, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx
	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIMx在CCR2上的预装载寄存器
	
	TIM_ARRPreloadConfig(TIM1, ENABLE); //使能TIMx在ARR上的预装载寄存器
	
	/* TIM3 enable counter */
	TIM_Cmd(TIM1, ENABLE);  //使能TIMx外设
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);  // 定时器1才有

}

void setDuty(uint8_t ch, uint16_t duty) {
		
	if (ch == 0) {
		
		TIM_SetCompare3(TIM2,duty);
		
	} else if (ch == 1) {
		
		TIM_SetCompare4(TIM2,duty);
	
	} else if (ch == 2) {
		
		//TIM_SetCompare4(TIM1,duty);
	
	} else if (ch == 3) {
		
		TIM_SetCompare1(TIM1,duty);
	
	} else if (ch == 4) {
		
		TIM_SetCompare2(TIM1,duty);
	
	} else if (ch == 5) {
		
		TIM_SetCompare3(TIM1,duty);
	
	} 
		
}

