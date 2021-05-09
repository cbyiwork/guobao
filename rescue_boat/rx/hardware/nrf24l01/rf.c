#include "nrf24l01.h"
#include "../uart/uart.h"
#include <string.h>

#define DATA_READY 1
#define DATA_NORCV 0
u8 g_rfBuf[32];
u8 g_isAdjust = 0;
u8 g_EngChan[4];
u16 g_EngDuty[4];
u16 g_EngDutyImp[4];

#define ENG_ADJ_STEP 50

u8 EnginesSet();
void EnginesDutyCalc();

typedef struct Rudder_Para{

	u8 isLeft;
	u8 powLeft;
	u8 isUp;
	u8 powUp;
	u8 isReverse;

}RudderPara;

RudderPara g_rudderPara;

u8 NrfDateRcvProc(u8* rBuf) {

	if (!NRF24L01_IRQ()) {
		if (!NRF24L01_RxPacket(rBuf)) {
			//dprint("receive data/r/n");
			return DATA_READY;
		}
		ClrRxFifo();
	}
	return DATA_NORCV;
}

void RcvDataHandle(u8 *buf) {

	u8 i = 0;
	u8 index = sizeof("gas:")-1;
	
	//dprint("%s\r\n",buf);
	
	if (!memcmp(buf,"gas:",index++)) {
		g_rudderPara.isLeft  = buf[index++];
		g_rudderPara.powLeft = buf[index++];
		g_rudderPara.isUp = buf[index++];
		g_rudderPara.powUp = buf[index++];	
		g_rudderPara.isReverse = buf[index++];
		// pow 为0 ~ 100
		//dprint("isLeft:%d, powLeft:%d, isUp:%d, powUp:%d, isRvs:%d\r\n", 
			//g_rudderPara.isLeft, g_rudderPara.powLeft,g_rudderPara.isUp,g_rudderPara.powUp,g_rudderPara.isReverse);
	}

}

void RfDataRcvLoop() {

	if (DATA_READY == NrfDateRcvProc(g_rfBuf)) {

		// 数据接收为即时，没有延时
		//dprint("receive data\r\n");
		RcvDataHandle(g_rfBuf);
		EnginesDutyCalc();
		g_isAdjust = 1;

	}

}


u32 g_enginesTimer = 0;
void AdjustEnginesLoop() {
	if (getTickMs(&g_enginesTimer) > 10) {
		refreshTimer(&g_enginesTimer);

		// 每隔10ms 设置一次马达驱动
		if (g_isAdjust) {
			//g_isAdjust = 0;
			g_isAdjust = EnginesSet();			
		}
		
	}
	
}

void initEngDuty() {

	u8 i = 0;
	for (i=0;i<2;i++) {
		g_EngDuty[i] = 1000;
		g_EngDutyImp[i] = 1000;
		setDuty(i, g_EngDutyImp[i]);
	}
	

}

u8 EnginesSet() {

	u8 ret = 0, i = 0;

	for (i=0;i<2;i++) {
		
		if (g_EngDuty[i] > g_EngDutyImp[i]) {
			
			g_EngDutyImp[i] += ENG_ADJ_STEP;
			if (g_EngDutyImp[i] >= g_EngDuty[i]) {
				g_EngDutyImp[i] = g_EngDuty[i];
			}else {
				ret = 1;
			}
			
		} else{
			g_EngDutyImp[i] -= ENG_ADJ_STEP;
			if (g_EngDutyImp[i] <= g_EngDuty[i]) {
				g_EngDutyImp[i] = g_EngDuty[i];
			}else {
				ret = 1;
			}

		}

		setDuty(i, g_EngDutyImp[i]);
		dprint("set ch[%d] duty[%d]\r\n",i,g_EngDutyImp[i]);
		
	}

	

	return ret;

}

void EnginesDutyCalc(){

	u16 acc = 0;

	if (0==g_rudderPara.isReverse) {
		if (0==g_rudderPara.isUp) {
			acc = 1500 + 5 * g_rudderPara.powUp;
			if (acc > 2000) {
				g_EngDuty[0] = 2000;
			} else {
				g_EngDuty[0] = acc;
			}
		} else {

			acc = 1500 - 5 * g_rudderPara.powUp;
			if (acc < 1000) {
				g_EngDuty[0] = 1000;
			} else {
				g_EngDuty[0] = acc;
			}
		}

		if (0==g_rudderPara.isLeft) {
			acc = 1500 + 5*g_rudderPara.powLeft;
			if (acc > 2000) {
				g_EngDuty[1] = 2000;
			} else {
				g_EngDuty[1] = acc;
			}
		} else {
			acc = 1500 - 5 * g_rudderPara.powLeft;
			if (acc < 1000) {
				g_EngDuty[1]  = 1000;
			} else {
				g_EngDuty[1] = acc;
			}
			
		}
		
	} else {

		if (0==g_rudderPara.isUp) {
			
			acc = 1500 + 5 * g_rudderPara.powUp;
			if (acc > 2000) {
				g_EngDuty[1] = 2000;
			} else {
				g_EngDuty[1] = acc;
			}
		} else {

			acc = 1500 - 10 * g_rudderPara.powUp;
			if (acc < 1000) {
				g_EngDuty[1] = 1000;
			} else {
				g_EngDuty[1] = acc;
			}
		}

		if (0==g_rudderPara.isLeft) {
			acc = 1500 + 5*g_rudderPara.powLeft;
			if (acc > 2000) {
				g_EngDuty[0] = 2000;
			} else {
				g_EngDuty[0] = acc;
			}
		} else {
			acc = 1500 - 5 * g_rudderPara.powLeft;
			if (acc < 1000) {
				g_EngDuty[0]  = 1000;
			} else {
				g_EngDuty[0] = acc;
			}
			
		}

	}

	//dprint("duty0: %d, duty1: %d\r\n",g_EngDuty[0],g_EngDuty[1]);
		
}



