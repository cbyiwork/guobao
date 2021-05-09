#include "timer.h"

void TIM3_Configuration(void)
{
	
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	/* TIM3 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	
	/* ---------------------------------------------------------------
	TIM3CLK ��PCLK1=36MHz
	TIM3CLK = 36 MHz, Prescaler = 7200, TIM3 counter clock = 5K,���ı�һ��Ϊ5K,���ھ�Ϊ10K
	--------------------------------------------------------------- */
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 0xffff; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms
	TIM_TimeBaseStructure.TIM_Prescaler =(7200-1); //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	
	/* Enables the Update event for TIM3 */
	//TIM_UpdateDisableConfig(TIM3,ENABLE); 	//ʹ�� TIM3 �����¼� 
	
	/* TIM IT enable */
	
	/* TIM3 enable counter */
	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx����
	
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


