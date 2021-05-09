#include "key.h"
#include <stdio.h>
#include "../oled/oled.h"
#include "../uart/uart.h"
#include "../timer/timer.h"

u8 g_chan = 0;
u8 g_srMode = 0;
u8 g_isKeyDown = 0;
u8 g_recorder = 0;

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
	
	dprint("Key1Up");
/*	g_chan ++;
	if (g_chan > 15){
		g_chan = 0;
	}
	SwChannal(g_chan);
	*/
}

void Key2Up(){
	dprint("Key2Up");
	/*
	if (g_srMode>0) {
		g_srMode = 0;
		OLED_ShowString(0,0, "NRF : R");
		dprint ("switch to receive mode\r\n");
	} else {
		g_srMode = 1;
		OLED_ShowString(0,0, "NRF : T");
		dprint ("switch to send mode\r\n");
	}
	OLED_Refresh_Gram();
	*/
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

u32 g_keyProcTimer = 0;

void KeyProc() {
	uint16_t key = KeyScan();

	if (g_isKeyDown==0) {
		if (key!=KEYNULL) {
			g_isKeyDown = 1;
			KeyDownProc(key);
			g_recorder = key;
			printf("key %d down\r\n",key);

		}
	} else {
		if (KEYNULL == key) {
			g_isKeyDown = 0;
			KeyUpProc(g_recorder);
			printf("key %d up\r\n",g_recorder);
		} else {
			
		}
	}

}


void KeyProcLoop() {
	if (getTickMs(&g_keyProcTimer)>50) {
		refreshTimer(&g_keyProcTimer);
		KeyProc();
	}
}


