#ifndef JSON_HANDLE_H
#define JSON_HANDLE_H

#include "main.h"
#include "cJSON.h"
#include "RTC_Format.h"

typedef enum JsonKey{
    JSON_KEY_IMEI,
    JSON_KEY_POWER,
    JSON_KEY_FAN,
    JSON_KEY_ODC,
    JSON_KEY_VAN_ERROR,
    JSON_KEY_TRIGGER_VAN,
    JSON_KEY_DP_HIGH,
    JSON_KEY_DP,
    JSON_KEY_DP_LOW,
    JSON_KEY_LED_BAR,
    JSON_KEY_TIME,
}JsonKey;
// reference in cJSON library
#define cJSON_OFFSET_BYTES 5

void jsHandle_Init(char *outputStr);
void jsHandle_SetIMEI(char *IMEI);
char* jsHandle_GetIMEI();
void jsHandle_SetDP(int DP);
int jsHandle_GetDP();
char* jsHandle_PrintObject();
void jsHandle_DeleteObject();
int jsHandle_Get_dpIndicatorBar();
void jsHandle_Set_dpIndicatorBar(uint8_t level);

void jsHandle_Set_dpLow(int dpLow);
int jsHandle_Get_dpLow();
void jsHandle_Set_dpHigh(int dpHigh);
int jsHandle_Get_dpHigh();
#endif