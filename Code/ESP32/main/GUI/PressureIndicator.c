#include "PressureIndicator.h"
#include "BoardParameter.h"
HC595 LedBar;

void PI_SetLevel(uint8_t level){
    if(level > 10) return;
    uint16_t indicator = INDICATOR_MAX_BITMASK;
    indicator <<= (INDICATOR_MAX_LEVEL-level);
    indicator &=INDICATOR_MAX_BITMASK;
    HC595_SetByteOutput(indicator);
    HC595_ShiftOut(NULL,2,1);
    HC595_ClearByteOutput(INDICATOR_MAX_BITMASK);
}

/**
 * @brief Tính toán mức LED cần hiển thị lên LED bar dựa theo áp suất min và max
 * Lưu ý giá trị LED nhỏ nhất là 2 để hiển thị được LED số 1 (Lý do hiện tại vẫn chưa rõ vì ảnh hưởng của hàm PI_SetLevel)
 * @param val mức LED hiển thị 
 * @return uint8_t 
 */
uint8_t PI_CalcLevelFromPressure(float val)
{
	uint32_t dpHigh = Brd_GetParamIntValue(INDEX_DP_HIGH);
	uint32_t dpLow = Brd_GetParamIntValue(INDEX_DP_LOW);
	uint32_t dpStep = (uint32_t)((dpHigh - dpLow)/10); 
	uint32_t a = dpLow + dpStep;
    if(val > dpHigh) return INDICATOR_MAX_LEVEL;
	for(uint8_t i=2; i <= 10;i++){
		if(a > val){
			return i; // return level of pressure indicator
		} else {
			a += dpStep;
		}
		if(a > dpHigh) return i;
		else if(a < dpLow) return 2;
	} 
	return 2;
}


void PI_Init()
{
    gpio_config_t cfg = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = HC595_DS_MASK |
                        HC595_OE_MASK |
                        HC595_CLK_MASK|
                        HC595_LATCH_MASK,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&cfg);
    HC595_AssignPin(&LedBar,GPIO_NUM_5,HC595_LATCH);
    HC595_AssignPin(&LedBar,GPIO_NUM_4,HC595_OE);
    HC595_AssignPin(&LedBar,GPIO_NUM_18,HC595_CLK);
    HC595_AssignPin(&LedBar,GPIO_NUM_23,HC595_DS);
    HC595_SetTarget(&LedBar);
    HC595_EnableOutput();
    HC595_ClearByteOutput(0xffffffff);
    HC595_ShiftOut(NULL,2,1); 
    for(uint8_t i=1;i<10;i++){
        PI_SetLevel(i);
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
    for(uint8_t i=10;i>1;i--){
        PI_SetLevel(i);
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
}