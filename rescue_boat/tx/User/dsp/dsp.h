#ifndef __DSP_H__
#define __DSP_H__
#include "stm32f10x.h"

#define FUNC_START 20
#define H_W 12
#define DUMMY 2
#define SIGNAL_W 16
#define NUM_LEN 8

#define SAFE_VOLTAGE_LEVEL 40000

void dsp_test(void);
void DspHorizontalBar(u8 x, u8 y,u8 dir, u8 pow);
void DspQrudStr(u8 x, u8 y, u8 percent);
void DspDirLeftRight(u8 x, u8 y,u8 dir, u8 pow);
void DspVerticalBar(u8 x, u8 y, u8 dir, u8 pow);
void DspDirUpDown(u8 x, u8 y,u8 dir, u8 pow);
void dspPacktsLoss(u8 loss);
void DspStd16Char(u8 x, u8 y, u8 ch);
void DspStd16String(u8 x, u8 y, u8 it, char* str);
void photo(u8 *p,u8 x,u8 y,u8 length,u8 width);
void LcdClear(u8 x, u8 y, u8 w, u8 h);
void DspCenterStd16String(u8 x, u8 y, u8 it, const char * str);
void dspBoatVoltage(u8 x, u8 y, u16 vol,u8 index);
void DspStdCNstring(u8 x, u8 y, char *cns[3], u8 len);
void DspCenterCNString(u8 x, u8 y, char *cns[3], u8 len);
void batteryLevelProc(void);


typedef enum DspMode
{
	MOD_OPR = 0,
	MOD_MENU
}DSP_MODE;

typedef enum OperationType{
	OPERA_H = 0,
	OPERA_L

}OperaType;





#endif

