#include "key.h"
#include <stdio.h>
#include "../oled/oled.h"
#include "../uart/uart.h"
#include "../sys/sys.h"
#include "../sys/delay.h"

u8 g_chan = 0;
u8 g_srMode = 0;
u8 g_isKeyDown = 0;
u8 g_recorder = 0;
u8 g_engReverse = 0;
void setSleepInt();
void enterSleepMode();

void KeyInit(void){
	GPIO_InitTypeDef gpio_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );	
	gpio_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	gpio_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio_InitStructure);
	
}

static uint16_t GetKeyState() {

	uint16_t keyPort = GPIO_ReadInputData(GPIOB);
	keyPort >>= 3;
	keyPort &= 0x7F;

	return keyPort;
}


u8 KeyScan(void) {
	return GetKeyState();
}

void SwChannal(u8 chan) {
	

}

void Key1Down(){
	dprint("Key1Down");
}

void Key2Down(){
	dprint("Key2Down");
}

void Key3Down(){
	dprint("Key3Down");
}

void Key4Down(){
	dprint("Key4Down");
}

void Key5Down(){
	dprint("Key5Down");
}

void Key6Down(){
	dprint("Key6Down");
}

void Key7Down(){
	dprint("Key7Down");
}

void Key1Up(){

	if (g_engReverse) {
		g_engReverse = 0;
	} else {
		g_engReverse = 1;
	}
	
}

void Key2Up(){
	
	if (g_chan > 15){
		g_chan = 0;
	}
	SwChannal(g_chan);
}

void Key3Up(){
	dprint("Key3Up");
}

void Key4Up(){
	dprint("Key4Up");
}

void Key5Up(){
	dprint("Key5Up");
}

void Key6Up(){
	dprint("Key6Up");
}

void Key7Up(){
	dprint("Key7Up");
}

void Key1LongPress() {
	dprint("key1long");
}

void Key2LongPress() {
	dprint("key2long");
}

void Key3LongPress() {
	dprint("key3long");
}

void Key4LongPress() {
	dprint("key4long");
}

void Key5LongPress() {
	dprint("key5long");
	setSleepInt();
	enterSleepMode();
}

void Key6LongPress() {
	dprint("key6long");
}

void Key7LongPress() {
	dprint("key7long");
}

void KeyDownProc(uint16_t key) {

	if (KEY1_VAL==key) {
		Key1Down();						
	} else if (KEY2_VAL==key) {		
		Key2Down();
	} else if (KEY3_VAL==key) {
		Key3Down();
	} else if (KEY4_VAL==key) {
		Key4Down();
	} else if (KEY5_VAL==key) {
		Key5Down();
	} else if (KEY6_VAL==key) {
		Key6Down();
	} else if (KEY7_VAL==key) {
		Key7Down();
	}

}

void KeyUpProc(uint16_t key) {

	if (KEY1_VAL==key) {
		Key1Up();						
	} else if (KEY2_VAL==key) {		
		Key2Up();
	} else if (KEY3_VAL==key) {
		Key3Up();
	} else if (KEY4_VAL==key) {
		Key4Up();
	} else if (KEY5_VAL==key) {
		Key5Up();
	} else if (KEY6_VAL==key) {
		Key6Up();
	} else if (KEY7_VAL==key) {
		Key7Up();
	}

}

void KeyLongPress(uint16_t key) {

	if (KEY1_VAL==key) {
		Key1LongPress();
	} else if (KEY2_VAL==key) {
		Key2LongPress();
	} else if (KEY3_VAL==key) {
		Key3LongPress();
	} else if (KEY4_VAL==key) {
		Key4LongPress();
	} else if (KEY5_VAL==key) {
		Key5LongPress();
	} else if (KEY6_VAL==key) {
		Key6LongPress();
	} else if (KEY7_VAL==key) {
		Key7LongPress();
	}
}

u32 g_keyProcTimer = 0;
u8 g_longPressCnt = 0;

void KeyProc() {

	// scan every 50ms
	uint16_t key = KeyScan();

	if (g_isKeyDown==0) {
		if (key!=KEYNULL) {
			g_isKeyDown = 1;
			KeyDownProc(key);
			g_recorder = key;
			//printf("key %d down\r\n",key);

		}
	} else {
		if (KEYNULL == key) {
			g_isKeyDown = 0;
			if (g_longPressCnt < 60){
				KeyUpProc(g_recorder);
			}
			g_longPressCnt = 0;
			//printf("key %d up\r\n",g_recorder);
		} else {
			g_longPressCnt ++;
			if (g_longPressCnt == 60) {
				KeyLongPress(g_recorder);
			}
		}
	}

}

void halt() {
	u16 cnt = 0;
	for (cnt = 0;cnt<60000;cnt++) {
		__NOP();
		__NOP();
		__NOP();
		__NOP();
		__NOP();
	}

}

void enterSleepMode() {

	u16 cnt = 0, i = 0;;
	OLED_Display_Off();
	PWR_LED_OFF();

	while (1) {

sleep:
		cnt = 0;
		PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
		RCC_Configuration();
/*
		for (i = 0;i<10;i++){

			halt();
			
			if (!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)) {
				cnt ++;
				if (cnt > 60) {
					goto wakeup;
				}
			} else {
				goto sleep;
			}
			//halt();
		}		*/

		for (i = 0;i<300;i++) {
			halt();
		}

		if (!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)) {
			break;
		}
		
	}

wakeup:	
	
	PWR_LED_ON();

	//RCC_Configuration();
	OLED_Display_On();
}

void setSleepInt() {

	GPIO_InitTypeDef gpio_InitStructure;
	EXTI_InitTypeDef exti_InitStructure;
	NVIC_InitTypeDef nvic_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO,ENABLE);

	// 2.GPIO初始化
  	gpio_InitStructure.GPIO_Pin = GPIO_Pin_8;
	gpio_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //浮空输入
	GPIO_Init(GPIOB,&gpio_InitStructure);

	// 3.设置EXTI线
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource8);  //将EXIT线9连接到PB9
	exti_InitStructure.EXTI_Line = EXTI_Line8;
	exti_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
 	exti_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;     //上升下降沿触发
  	exti_InitStructure.EXTI_LineCmd = ENABLE;//使能中断线
  	EXTI_Init(&exti_InitStructure);//初始化中断

	 // 4.中断向量
	nvic_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	nvic_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	nvic_InitStructure.NVIC_IRQChannelSubPriority = 3;
	nvic_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_InitStructure);
	

}


void EXTI9_5_IRQHandler() {
	
	if (EXTI_GetITStatus(EXTI_Line8)!=RESET) {
		dprint("wake up\r\n");
	}
	EXTI_ClearITPendingBit(EXTI_Line8);

}

void KeyProcLoop() {
	if (getTickMs(&g_keyProcTimer) > 50) {
		refreshTimer(&g_keyProcTimer);
		KeyProc();
	}
}

