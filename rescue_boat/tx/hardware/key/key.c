#include "key.h"
#include <stdio.h>
#include "../oled/oled.h"
#include "../uart/uart.h"
#include "../sys/sys.h"
#include "../sys/delay.h"

u8 g_chan = 0;
u8 g_srMode = 0;
u8 g_isKeyDown = 0;
uint16_t g_recorder = 0;
u8 g_engReverse = 0;
u32 g_lowPowerTimer = 0;
u16 g_lowPowerCnt = 0;
u8 g_isPowerOn = 0;
u8 g_pwrkey_release = 0;

#define LONG_PRESS_THRESHOLD 30


void setSleepInt();
void enterSleepMode();

void KeyInit(void){
	GPIO_InitTypeDef gpio_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );	
	//gpio_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	gpio_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_3;
	gpio_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &gpio_InitStructure);
	
}

static uint16_t GetKeyState() {

	uint16_t keyPort = ~ GPIO_ReadInputData(GPIOB);
	//keyPort >>= 8;
	keyPort &= 0x308;                // PB9 PB8 PB3

	//dprint ("%s, keyPort[%x]\n", __FUNCTION__,keyPort);

	return keyPort;
}


uint16_t KeyScan(void) 
{
	return GetKeyState();
}

void SwChannal(u8 chan) {
	

}

void KeyDownProc(uint16_t key) 
{
    if (key & 0x200)   // power key 
    {
        // PWR key
        
    } else if (key & 0x100) {

    } else if (key & 0x08)  {

    }

}

void KeyUpProc(uint16_t key) 
{

    if (key & 0x200)   // power key 
    {
        // PWR key
        dprint("key 9 up\n");
        
    } else if (key & 0x100) {
		dprint("power key up\n");
		g_pwrkey_release = 1;

    } else if (key & 0x08)  {
		
		dprint("sw key up\n");
		swMode();
    }

}

void PwrKeyLongPress()
{
	if (!g_pwrkey_release) {
		g_pwrkey_release = 1;
		return;
	}
    enterSleepMode();

}

void KeyLongPress(uint16_t key) {

	if (key & 0x200)   // power key 
    {
        // PWR key
        //PwrKeyLongPress();
        dprint("key 9 long press\n");
        
    } else if (key & 0x100) {

		dprint("power down\n");
    	PwrKeyLongPress();

    } else if (key & 0x08)  {
        //swMode();
        dprint("sw key long press\n");
    }
}

u32 g_keyProcTimer = 0;
u8 g_longPressCnt = 0;

void KeyProc() {

	// scan every 50ms
	uint16_t key = KeyScan();

	if (g_isKeyDown == 0) {
		if (key != KEYNULL) {
			g_isKeyDown = 1;
			KeyDownProc(key);
			g_recorder = key;
			dprint("key %d down\r\n",key);

		}
	} else {
		if (KEYNULL == key) {
			g_isKeyDown = 0;
			if (g_longPressCnt < LONG_PRESS_THRESHOLD){
				KeyUpProc(g_recorder);
			}
			g_longPressCnt = 0;
			dprint("key %d up\r\n",g_recorder);
		} else {
			g_longPressCnt ++;
			if (g_longPressCnt == LONG_PRESS_THRESHOLD) {
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

    u16 i = 0;;
    OLED_Display_Off();
    PWR_LED_OFF();

    while(1) {
        delay_ms(100);
    };
}

void checkPowerOn(void)
{
    uint16_t counter = 0;
    PWR_LED_OFF();

    while (1) {

        if (!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)) {
            counter ++;
            if (counter > 200) {
                PWR_LED_ON();
                break;
            }
        } else {
			counter = 0;
		}

        //delay_second(10);
        delay_ms(10);

		dprint("count: %d\n", counter);

    }
}


void setSleepInt() {

	GPIO_InitTypeDef gpio_InitStructure;
	EXTI_InitTypeDef exti_InitStructure;
	NVIC_InitTypeDef nvic_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO,ENABLE);

	// 2.GPIO??????
  	gpio_InitStructure.GPIO_Pin = GPIO_Pin_8;
	gpio_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //????????
	GPIO_Init(GPIOB,&gpio_InitStructure);

	// 3.????EXTI??
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource8);  //??EXIT??9??????PB9
	exti_InitStructure.EXTI_Line = EXTI_Line8;
	exti_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
 	exti_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;     //??????????????
  	exti_InitStructure.EXTI_LineCmd = ENABLE;//??????????
  	EXTI_Init(&exti_InitStructure);//??????????

	 // 4.????????
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

void KeyProcLoop() 
{
	if (getTickMs(&g_keyProcTimer) > 50) {
		refreshTimer(&g_keyProcTimer);
		KeyProc();
	}
}

void enterLowpowerMode()
{
    g_lowPowerCnt ++;
	if (g_lowPowerCnt > 60*20) {
		g_lowPowerCnt = 0;
		enterSleepMode();
	}
}


