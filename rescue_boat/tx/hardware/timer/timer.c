#include "timer.h"

void TIM3_Configuration(void)
{
	
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	/* TIM3 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	
	/* ---------------------------------------------------------------
	TIM3CLK 即PCLK1=36MHz
	TIM3CLK = 36 MHz, Prescaler = 7200, TIM3 counter clock = 5K,即改变一次为5K,周期就为10K
	--------------------------------------------------------------- */
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 0xffff; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =(7200-1); //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	/* Enables the Update event for TIM3 */
	//TIM_UpdateDisableConfig(TIM3,ENABLE); 	//使能 TIM3 更新事件 
	
	/* TIM IT enable */
	
	/* TIM3 enable counter */
	TIM_Cmd(TIM3, ENABLE);  //使能TIMx外设
	
}

u32 getTickMs(u32* tmr) {
	u32 interval = 0;
	//interval = SysTick->VAL - *tmr;
	//interval =  *tmr - SysTick->VAL;
	interval = TIM_GetCounter(TIM3) - *tmr;
	//*tmr = TIM_GetCounter(TIM3);
	return (interval/10);

}

void refreshTimer(u32* tmr) {
	*tmr = TIM_GetCounter(TIM3);
}


