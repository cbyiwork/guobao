/**
  ******************************************************************************
  * @file    main.c
  * $Author: wdluo $
  * $Revision: 67 $
  * $Date:: 2012-08-15 19:00:29 +0800 #$
  * @brief   主函数.
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, ViewTool</center>
  *<center><a href="http:\\www.viewtool.com">http://www.viewtool.com</a></center>
  *<center>All Rights Reserved</center></h3>
  * 
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "hw_config.h"
#include "usbio.h"
//#include "../sys/sys.h"
#include "../iap/iap.h"
#include "../Enc_Dec/encrypto.h"
#include "../stmflash/stmflash.h"
#include "../hardware/oled/oled.h"
#include "../hardware/nrf24l01/nrf24l01.h"
#include "../hardware/uart/uart.h"
#include "../sys/sys.h"
#include "../sys/delay.h"
#include "../hardware/key/key.h"
#include "../hardware/adc/adc.h"
#include "../hardware/timer/timer.h"

void checkPowerOn(void);

void PortInit(void)
{
	GPIO_InitTypeDef gpio_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
	
	//RCC->APB2ENR |= 1<<2;   // port A 时钟使能
	//RCC->APB2ENR |= 1<<3;//先使能外设PORTB时钟
	//RCC->APB2ENR |= 1<<5;		//enable port D clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
	//GPIOA->CRL&=0xFFFFFFF0; 			//PA1
	//GPIOA->CRL|=0x00000008;	
	gpio_InitStructure.GPIO_Pin = GPIO_Pin_15;
	gpio_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA,&gpio_InitStructure);

	gpio_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	gpio_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

	GPIO_Init(GPIOB,&gpio_InitStructure);
	/*pb10: clk, pb11: sda, pb12: pwr, pb14: rst, pb15: cs*/
	//LCD_PWR_SET();
	//PWR_LED_ON();
	
	
}

void led_twinkle()
{
     static uint8_t state = 0;
	   
	   if (state > 0) {
		    state = 0;
			  PWR_LED_OFF();
		 } else {
			   state = 1;
		     PWR_LED_ON();
		 }
}

void SecondHandler()
{
    enterLowpowerMode();
    //led_twinkle();
}

unsigned int g_secondTimer = 0;
void SecondHandlerLoop() {
	if (getTickMs(&g_secondTimer) > 1000) {
		refreshTimer(&g_secondTimer);
		SecondHandler();
	}
}


extern u8 pwr;
int main(void)
{
	u8 t =0;
	u8 key = 0;
	u8 rfBuf[33];
	u8 pow = 0;
	
	Stm32_Clock_Init(9);
	delay_init(72);
	AdcInit();
	//uart1_init(72,115200);
	USART1_Init(115200);

	dprint("%s, %s\r\n",__DATE__, __TIME__);
	
	PortInit();
	//NRF_init();
	// system tick timer
	TIM2_Configuration();
    TIM3_Configuration(20000);
	KeyInit();
	//NRF24L01_Init();
	//EXTI_PB2_Init();
	OLED_Init();			//初始化液晶   
	/*
	t = NRF24L01_Check();
	if (t!=0) {
		//OLED_ShowString(0,0, "NRF ERROR");
		dprint("NRF ERROR");
	} else {
		//OLED_ShowString(0,0, "NRF OK");
		dprint("NRF OK");
	}*/

	//g_srMode = 1;
	//TX_Mode();

	//NrfPower(3);

	//dsp_test();

	setSleepInt();

    checkPowerOn();

	//OLED_Refresh_Gram();
	//while(KEYNULL==KeyScan());	
	while (1){

		KeyProcLoop();
		//getAdcConvertResult();
		checkCtlStickLoop();
		//checkCtlStick();
		//dprint("run, %d\r\n",pwr);
		//bar_test();
		refreshScreen();
        SecondHandlerLoop();
        AdjustEnginesLoop();
		//rfSendInfoProc();
		//delay_ms(100);		
		//printf("key: %d\r\n",key);
		//dprint("run\r\n");
		
	};

	
	//LCD_Init();
	//OLED_Init();
/*	
	//printf("%s, %s\r\n",__DATE__, __TIME__);   
 	OLED_ShowString(0,0, "0.96' OLED TEST");  
 	OLED_ShowString(0,16,"ATOM@ALIENTEK");  
 	OLED_ShowString(0,32,"2010/06/3");  

 	OLED_ShowString(0,48,"ASCII:");  
 	OLED_ShowString(63,48,"CODE:");  
	OLED_Refresh_Gram();	 
	t=' ';  
	
	while(1) 
	{		
		OLED_ShowChar(48,48,t,16,1);//显示ASCII字符	   
		OLED_Refresh_Gram();
		t++;
		if(t>'~')t=' ';
		OLED_ShowNum(103,48,t,3,16);//显示ASCII字符的码值 
		delay_ms(300);
		//LED0=!LED0;
	}
*/	
	//USB_Interrupts_Config();
	//Set_USBClock();
	//USB_Init();
	//LCD_Print(0, 0, "搜索卫星...",TYPE16X16,TYPE8X16);
	//LCD_PutPixel(0,0);
	//LCD_Print(0, 0, "搜索卫星...",TYPE16X16,TYPE8X16);

	//while(1){}
/*
	while(1){
		LCD_Print(0, 0, "搜索卫星...",TYPE16X16,TYPE8X16);
		LCD_Print(0, 16, "abcd09876",TYPE16X16,TYPE8X16);
		LCD_Print(16, 32, "efg12345",TYPE16X16,TYPE6X8);
		Draw_BMP(32,48,nonside);
		Draw_BMP(48,48,nonside);
		Draw_BMP(64,48,nonside);

	}
	*/
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  报告在检查参数发生错误时的源文件名和错误行数
  * @param  file 源文件名
  * @param  line 错误所在行数 
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* 用户可以增加自己的代码用于报告错误的文件名和所在行数,
       例如：printf("错误参数值: 文件名 %s 在 %d行\r\n", file, line) */

    /* 无限循环 */
    while (1)
    {
    }
}
#endif

/*********************************END OF FILE**********************************/
