#include "moto.h"
#include "../hardware/key/key.h"
#include "../hardware/adc/adc.h"
#include "../hardware/timer/timer.h"
#include "../hardware/oled/oled.h"
#include "../hardware/uart/uart.h"

//u8 g_isAdjust = 0;
extern u8 g_paraChanged;
extern u16 g_EngDuty[4];

u32 g_enginesTimer = 0;
u16 g_EngDutyImp[4] = {0};
u8 EnginesSet(void);
void setDuty(uint8_t ch, uint16_t duty);

void AdjustEnginesLoop() 
{
    if (getTickMs(&g_enginesTimer) > 10) {
        refreshTimer(&g_enginesTimer);

        // 每10ms调整一次马达输出
        if (g_paraChanged) {
            //g_isAdjust = 0;
            g_paraChanged = EnginesSet();			
        }
    }
    
}

u8 EnginesSet(void) 
{

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

void setDuty(uint8_t ch, uint16_t duty) 
{
		
	if (ch == 0) {
		
		TIM_SetCompare1(TIM3,duty);
		
	} else if (ch == 1) {
		
		TIM_SetCompare2(TIM3,duty);
	
	} 
		
}

