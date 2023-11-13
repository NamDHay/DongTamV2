#ifndef PI_H_
#define PI_H_

#include "74HC595.h"
#include "freertos/FreeRTOS.h"
#define INDICATOR_MAX_BITMASK 0x01ff
#define INDICATOR_MIN_BITMASK 0x0100
#define INDICATOR_MAX_LEVEL 10

#define HC595_OE_MASK (1ULL<<GPIO_NUM_4)
#define HC595_LATCH_MASK (1ULL<<GPIO_NUM_5)
#define HC595_CLK_MASK (1ULL<<GPIO_NUM_18)
#define HC595_DS_MASK (1ULL<<GPIO_NUM_23)

void PI_SetLevel(uint8_t level);
void PI_Init();
uint8_t PI_CalcLevelFromPressure(float val);
#endif