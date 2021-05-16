#include "dsp.h"
#include "../../hardware/oled/oled.h"
#include "font.h"
#include "../../hardware/uart/uart.h"
#include <string.h>
//#include "../../hardware/rf/rf.h"
u8 ram_index_x = 0;
u8 ram_index_y = 0;

extern u8 g_lastIsLeft;
extern u8 g_lastLeftPwr;
extern u8 g_lastIsUp;
extern u8 g_lastUpPwr;
extern u8 g_pktLossRate;
extern u8 g_engReverse;
extern u8 g_dspMod;
u8 g_oprMode = OPERA_L;
u8 g_isDspOprMod = 1;
u8 g_isDspTwinkle = 0;
u8 g_tkDispear = 0;
u8 g_batDispear = 0;
extern ChineseChar font_chinest[CN_CHAR_NUM];
extern uint32_t g_batVoltage;
extern u8 g_batWarning[2];
extern u8 g_isDspDevBat; 
extern u8 g_batDspIndex;
extern u8 g_batBalanceError;
extern u8 g_isFlyUnLock;

void location(u8 x, u8 y) 
{
	ram_index_x = x;
	ram_index_y = y;
}

void LCD(u8 data) {
	OLED_GRAM[ram_index_x++][ram_index_y] = data;
	if (ram_index_x >= 128) {
			ram_index_x = 0;
	}
}

void LCD_clean(u8 x,u8 y,u8 length,u8 width)// 清空指定区域
{
	u8 i,j;
  for(i=0;i<width;i++)
  {
		location(x,i+y);
		for(j=0;j<length;j++)
	  LCD(0);
  }
}

void photo(u8 *p,u8 x,u8 y,u8 width,u8 height)//ÏÔÊ¾Ö¸¶¨´óÐ¡µÄÍ¼Æ¬
{
	u8 i,j;
	for(i=0;i<height;i++)
	{
		location(x,i+y);
		for(j=0;j<width;j++)
	  	LCD(*p),p++;
	}  	
}

void LcdClear(u8 x, u8 y, u8 w, u8 h) {
	
	u8 i,j;
	for(i=0;i<h;i++)
	{
		location(x,i+y);
		for(j=0;j<w;j++)
	  	LCD(0x00);
	}  	
}

#define FONT_W 8
void DspVolNum(u8 x, u8 y, u8 num) {
	u8* pf;
	if (num == '%') {
		num = 10;
	}
	pf = (u8*)font_num16[num];
	photo(pf,x,y,FONT_W,2);
	
}

void DspStd16Char(u8 x, u8 y, u8 ch) {
	u8* pf;
	pf = (u8*)font_char16[ch-' '];	
	photo(pf, x,y,FONT_W,2);

}

void DspStd16String(u8 x, u8 y, u8 it, char* str) {
	while (*str != '\0') {
		if (*str != ' ') {
			DspStd16Char(x, y,*str);
		}
		x += (FONT_W+it);
		str ++;
	}
}

void DspCenterStd16String(u8 x, u8 y, u8 it, const char * str) {

	u16 strLen = 0;
	char * dstr =  (char*)str;
	while (*dstr != '\0') {
		strLen += (FONT_W+it);
		dstr ++;
	}

	if (x+strLen >= 127) {
		x = 0;
	} else {
		x = (127-(x+strLen))/2;
	}

	DspStd16String(x, y, it, (char*)str);

}

void DspCenterCNString(u8 x, u8 y, char *cns[3], u8 len){
	x = ((128-x)-(len*16))/2;
	DspStdCNstring(x,y,cns,len);
}

void DspVolNumStr(u8 x, u8 y, u8 percent) {

	//u8 idx = x;
	LcdClear(x, y, FONT_W*4, 2);
	if (percent >= 100) {
		DspVolNum(x,y,1);
		//percent%=100;	
		x+=FONT_W;
	} 

	if (percent >= 10) {
		//x+=FONT_W;
		DspVolNum(x,y,(percent%100)/10);
		x+=FONT_W;
	}
	//x+=FONT_W;
	DspVolNum(x,y,percent%10);
	x+=FONT_W;
	DspVolNum(x,y,'%');
}

void DspBatteryLevel(u8 x, u8 y, u8 level) {
	if (level>=4) {
		photo((u8*)bat_full,x,y,18,2);
	} else if(level>=3) {
		photo((u8*)bat_80,x,y,18,2);
	} else if(level>=2) {
		photo((u8*)bat_50,x,y,18,2);
	} else if(level>=1) {
		photo((u8*)bat_20,x,y,18,2);
	} else {
		photo((u8*)bat_low,x,y,18,2);
	}
}
/*
void batteryLevelProc() {
	u8 batLevel = 0;
	if (g_batVoltage > 3900) {
		batLevel = 4;
	} else if(g_batVoltage > 3700) {
		batLevel = 3;
	} else if(g_batVoltage > 3550) {
		batLevel = 2;
	} else if(g_batVoltage > 3300) {
		batLevel = 1;
	} else {
		batLevel = 0;
	}

	if ((batLevel > 0) || (g_batDispear == 0)) {
		DspBatteryLevel(FUNC_START+H_W+DUMMY+SIGNAL_W+DUMMY+NUM_LEN*4+DUMMY+16+DUMMY,0,batLevel);
	}

}*/

u32 getRevert(u32 dat) {

	u8 i = 0;
	u32 ret = 0;
	for(i=0;i<24;i++) {

		if ((dat>>i)&0x01) {
			ret |= (1<<(23-i));
		}
		
	}
	return ret;

}

void DspVerticalBar(u8 x, u8 y, u8 dir, u8 pow) {

	u8 rows = 0, i=0;
	u8 buf[24];
	u32 tmp = 0;

	memcpy(buf, vertical_bar,24);
	
	if (dir == 0) {
		rows = pow/4;
		if (rows > 24) rows = 24;		
		//tmp = (((1<<rows)-1)<<(24-rows));
		tmp = (0xffffff>>(24-rows));
		tmp = getRevert(tmp);
		
		for (i=0;i<4;i++) {
			buf[i+8]|=(tmp>>16)&0xff;
			buf[i+4]|=((tmp>>8)&0xff);
			buf[i]|=(tmp&0xff);
		}
	} else {
		rows = pow/4;
		if (rows > 24) rows = 24;	
		tmp = (0xffffff>>(24-rows));
		for (i=0;i<4;i++) { 
			buf[i+12+8]|=(tmp>>16);
			buf[i+12+4]|=((tmp>>8)&0xff);
			buf[i+12]|=(tmp&0xff);
		}
	}

	photo(buf,x,y,4,6);

}


void DspStdCNchar(u8 x, u8 y, char * cn) {

	u8 i = 0;
	
	for (i=0;i<CN_CHAR_NUM;i++) {
		if (!memcmp(cn,font_chinese[i].str,4)) {
			break;
		}
	}

	//dprint("cn[%s][%x%x], %d, ft[%s][%x%x], \r\n", cn, cn[0], cn[1],i, font_chinese[i].str,font_chinese[i].str[0],font_chinese[i].str[1]);
	
	if (i >= CN_CHAR_NUM) {
		dprint("no such chinest\r\n");
		return;
	}

	photo((u8*)font_chinese[i].font, x, y, 16, 2);

}

void DspStdCNstring(u8 x, u8 y, char *cns[3], u8 len) {
	u8 i = 0;
	/*
	for (i=0;i<len;i++) {
		//dprint("%s\r\n", cns[i]);
		DspStdCNchar(x, y, cns[i]);
		x += 16;
	}*/
	
	while (cns[i][0] != '\0') {
		DspStdCNchar(x, y, cns[i]);
		x += 16;
		i ++;
	}
}


void DspHorizontalBar(u8 x, u8 y,u8 dir, u8 pow) {

	u8 buf[48];
	u8 rows = 0, i=0;

	rows = pow/4;
	if (rows>24) rows = 24;

	memcpy(buf, horizontal_bar, 48);

	if (dir > 0) {

		for (i=0;i<rows;i++) {
			buf[24-i] |= 0x3c;
		}

	} else {
	
		for (i=0;i<rows;i++) {
			buf[i+24] |= 0x3c;
		}
	}
	
	photo(buf,x,y,48,1);
	
}

void DspQrudNum(u8 x, u8 y, u8 num) {
	u8* pf;
	if (num == '%') {
		num = 10;
	}
	pf = (u8*)font_num8[num];
	photo(pf,x,y,6,1);
	
}

void DspQrudStr(u8 x, u8 y, u8 percent) {

	//u8 idx = x;
	LcdClear(x,y,24,1);
	if (percent >= 100) {
		percent = 100;
		DspQrudNum(x,y,1);
		//percent%=100;
		x+=6;
	}
	if (percent>=10)
	{
		DspQrudNum(x,y,(percent%100)/10);
		x+=6;
	}
	DspQrudNum(x,y,percent%10);

	if(percent>0){
		x+=6;
		DspQrudNum(x,y,'%');
	}
}

void dsp32FontNumber(u8 x, u8 y, char num) {

	if (num == '.') {
		num = 10;
	} else if ((num == 'V' || num == 'v')) {
		num = 11;
	} else if ((num == 'W')|| (num == 'w')) {
		num = 12;
	}else {
		num = num - '0';
	}
	photo((u8*)font_char32[num],x,y,16,4);
}

void dsp32FontString(u8 x, u8 y, char *str) {

	while (*str != '\0') {
		dsp32FontNumber(x, y, *str);
		str ++;
		x += 16;
	}

}

void dspBoatVoltage(u8 x, u8 y, u16 vol,u8 index) {
	char buf[64];

	if (vol < 10000) {
		if (index == 0) {
			sprintf(buf, "%d.%d%dv", vol/1000, (vol%1000)/100, (vol%100)/10);
		} else {
			sprintf(buf, "%d.%d%dw", vol/1000, (vol%1000)/100, (vol%100)/10);
		}
	} else {
		if (index == 0) {
			sprintf(buf, "%d%d.%dv", vol/10000, (vol%10000)/1000,(vol%1000)/100);
		} else {
			sprintf(buf, "%d%d.%dw", vol/10000, (vol%10000)/1000,(vol%1000)/100);
		}
		
	}
	dsp32FontString(x, y, buf);
}



void DspDirLeftRight(u8 x, u8 y,u8 dir, u8 pow) {

	u8* pf;

	if (!pow) {
		LcdClear(x,y,6,1);
		return;
	}

	if (dir > 0) {
		pf = (u8*)font_num8[13];
	} else {
		pf = (u8*)font_num8[14];
	} 
	
	photo(pf,x,y,6,1);
 	
}

void DspDirUpDown(u8 x, u8 y,u8 dir, u8 pow) {

	u8* pf;

	if (!pow) {
		LcdClear(x,y,6,1);
		return;
	}

	if (dir > 0) {
		pf = (u8*)font_num8[12];
	} else {
		pf = (u8*)font_num8[11];
	} 

	photo(pf,x,y,6,1);
 	
}

extern u32 g_voltage;
u8 g_isDspDevBat = 0;
void batteryInfoDsp() 
{
    /*
	if (g_batDspIndex == 0) 
    {
		if (!g_batWarning[0]) 
        {
			dspBoatVoltage(30,2,g_boatVoltage[0],0);
		} else {
            
			if (g_isDspDevBat) 
            {
				dspBoatVoltage(30,2,g_boatVoltage[0],0);
			}
		}
	} else {
		if (!g_batWarning[1]) 
        {
			dspBoatVoltage(30, 2, g_boatVoltage[1],1);
            
		} else 
        {
			if (g_isDspDevBat) 
            {
				dspBoatVoltage(30,2,g_boatVoltage[1],1);
			}
		}
	}*/

    //if (!g_batWarning) 
    if (g_voltage > SAFE_VOLTAGE_LEVEL)
    {
		dspBoatVoltage(30,2,g_voltage,0);
	} else {
        
		if (g_isDspDevBat) 
        {
			dspBoatVoltage(30,2,g_voltage,0);
		}
	}

    /*
	if (g_batBalanceError) {
		photo((u8*)font_bat_balance_err,120,2,5,4);
	}*/

	// for test 
	//photo((u8*)font_bat_balance_err,120,2,5,4);


}

void operaModeDsp() 
{

    if (OPERA_H == g_oprMode) {
        photo((u8*)icon_h,FUNC_START,0,12,2);
    } else if (OPERA_L == g_oprMode) {
        //photo((u8*)icon_l,FUNC_START,0,12,2);icon_s
        photo((u8*)icon_l,FUNC_START,0,12,2);
    } 
}

void BuildScreenContent() {

    LcdClear(0,0,128,8);


    //photo((u8*)icon_h,FUNC_START,0,12,2);
    operaModeDsp();

    batteryInfoDsp();

    // ver 2.7 盲驴庐忙鈥澛姑寂捗ぢ嘎嵜モ?犅嵜λ溌久ぢ好ぢ嘎⒚ヅ掆?γр劉戮氓藛鈥犆β??
    /*
    if (g_rfModuleErr) {
    	photo((u8*)icon_signal_err,FUNC_START+H_W+DUMMY,0,16,2);
    }else {
    	photo((u8*)icon_signal,FUNC_START+H_W+DUMMY,0,16,2);
    }*/
    //DspBatteryLevel(FUNC_START+H_W+DUMMY+SIGNAL_W+DUMMY+NUM_LEN*4+DUMMY,0,4);
    //batteryLevelProc();

    DspHorizontalBar(40,7,g_lastIsLeft,g_lastLeftPwr);
    DspQrudStr(90,7,g_lastLeftPwr);
    DspDirLeftRight(40-6-1,7,g_lastIsLeft,g_lastLeftPwr);

    DspVerticalBar(3,1,g_lastIsUp,g_lastUpPwr);
    DspQrudStr(8,6,g_lastUpPwr);
    DspDirUpDown(8,1,g_lastIsUp,g_lastUpPwr);

    //dspPacktsLoss(g_pktLossRate);

    /*if (g_engReverse>0) {
    	photo((u8*)font_revert,127-16, 6,16,2);
    }*/
    /*
    if (g_engReverse == 0) {
    	photo((u8*)font_revert,127-16, 6,16,2);
    }*/

    /*
    if (g_isUnlockFC) {
    	photo((u8*)isLockIcon, 36, 3, 55, 2);
    }

    if (!g_isFlyUnLock) {
    	photo((u8*)icon_lock, FUNC_START+H_W+DUMMY+SIGNAL_W+DUMMY+NUM_LEN*4+DUMMY, 0, 16, 2);
    } */

}

u32 g_refreshScreenTimer = 0;
u32 g_oprDspTwinkleTimer = 0;
void refreshScreen() {
	//dprint("refresh\r\n");
	//u32 interval = getTickMs(&g_refreshScreenTimer);
	//dprint("interval %x\r\n",interval);
	if (getTickMs(&g_refreshScreenTimer) > 100) {
		refreshTimer(&g_refreshScreenTimer);
		BuildScreenContent();
		OLED_Refresh_Gram();
		//clearUnlock();
		//dprint("refresh\r\n");
	}

    if (getTickMs(&g_oprDspTwinkleTimer) > 500) {
        refreshTimer(&g_oprDspTwinkleTimer);

        if (g_isDspDevBat) {
            g_isDspDevBat = 0;
        } else {
            g_isDspDevBat = 1;
        }

    }

}

void swMode()
{
    if (OPERA_H == g_oprMode) {
        g_oprMode = OPERA_L;
    } else {
        g_oprMode = OPERA_H;
    }
}

u16 g_EngDuty[4] = {0};

void EnginesDutyCalc(uint8_t isReverse, uint8_t isUp, uint8_t powUp, uint8_t isLeft, uint8_t powLeft)
{

	u16 acc = 0;
    u16 step = 0;
    u16 max = 0;
    u16 min = 0;

    if (OPERA_L == g_oprMode) {
        step = 3;
        max = 1800;
        min = 1200;
    } else {
        step = 5;
        max = 2000;
        min = 1000;
    }

    if (0 == isUp) {
        acc = 1500 + step * powUp;
        if (acc > max) {
            g_EngDuty[0] = max;
            g_EngDuty[1] = max;
        } else {
            g_EngDuty[0] = acc;
            g_EngDuty[1] = acc;
        }
    } else {

        acc = 1500 - step * powUp;
        if (acc < min) {
            g_EngDuty[0] = min;
            g_EngDuty[1] = min;
        } else {
            g_EngDuty[0] = acc;
            g_EngDuty[1] = acc;
        }
    }

    if (0 == isLeft) {
        //acc = 1500 + 5 * powLeft;
        acc = step * powLeft;
        g_EngDuty[0] += acc;
        if (g_EngDuty[0] > max) {
            g_EngDuty[0] = max;
        }

        g_EngDuty[1] -= acc;
        if (g_EngDuty[1] < min) {
            g_EngDuty[1] = min;
        }
        
    } else {
        acc = step * powLeft;
        g_EngDuty[0] -= acc;
        if (g_EngDuty[0] < min) {
            g_EngDuty[0] = min;
        }

        g_EngDuty[1] += acc;
        if (g_EngDuty[1] > max) {
            g_EngDuty[1] = max;
        }

    }

    //dprint("duty0: %d, duty1: %d\r\n",g_EngDuty[0],g_EngDuty[1]);
}



