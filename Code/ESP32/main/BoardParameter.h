#ifndef INC_BOARDPARAMETER_H_
#define INC_BOARDPARAMETER_H_

#include "./main.h"
#include "RTC_Format.h"
#include "esp_err.h"

#define MAX_VAN 16
#define PARAM_STRING_ORDER_OFFSET 14

/*
This enum show full index of param in real table but the order is changed to make it easy to manage data.
From INDEX_DP_LOW to INDEX_SERV_RUN_HOURS_ALARM have data type integer, so we can loop these index to assign HighLimit value, 
LowLimit value and Default value; the other have data type struct or string which have special handle
In BoardParameter.c has paramMaxLimit array and paramMinLimit array which is mapped with this enum ParamIndex
In GUI.c has paramText pointer char* array which is mapped to this enum ParamIndex
*/ 
typedef enum {
    INDEX_START_PARAM,
    // unit is none
    INDEX_TOTAL_VAN,
    INDEX_DOWN_TIME_CYCLE,
    INDEX_ODC_CLEAN_MODE,
    INDEX_TEST_MODE,
    INDEX_DISPLAY_CONTRAST,
    // unit is Pa
    INDEX_DP_LOW,
    INDEX_DP_HIGH,
    INDEX_DP_WARN,
    INDEX_ODC_HIGH,
    INDEX_ODC_LOW,
    // unit is ms
    INDEX_PULSE_TIME,
    // unit is s
    INDEX_INTERVAL_TIME,
    INDEX_CYCLE_INTERVAL_TIME,
    // unit is h
    INDEX_OPERATE_HOURS,
    INDEX_SERV_RUN_HOURS,
    INDEX_SERV_RUN_HOURS_ALARM,
    // special param to handle with string value
    INDEX_STRING_PARAM_OFFSET,
    INDEX_LANGUAGE,
    INDEX_DISPLAY_RANGE,
    INDEX_PARAM_CODE,
    INDEX_TECH_CODE,
    INDEX_DP_MODE,
    INDEX_TRIG_VAN,
    INDEX_END_PARAM,
}ParamIndex;



typedef struct BoardParameter
{
    //Handle with integer, no unit
    uint8_t totalVan;
    uint8_t downTimeCycle;
    uint8_t cleanMode;
    uint8_t contrast;
    uint8_t testMode;
    //Handle with integer, have unit
    uint16_t dpHigh;
    uint16_t dpLow;
    uint16_t dpWarn;
    uint16_t odcHigh;
    uint16_t odcLow;
    uint16_t pulseTime;
    uint16_t intervalTime;
    uint16_t cycIntvTime;
	uint16_t operateHours;
    uint16_t servRunHours;
    uint16_t servAlarm;
    //Handle with string, no unit
    uint8_t language;
    uint8_t disRange;
    uint8_t paramCode;
    uint8_t techCode;
    uint8_t dpMode;
    uint8_t trigVan;
    float pressure;
    RTC_t RTCtime;
}BoardParameter;



uint16_t Brd_GetMaxLimit(uint8_t index);
uint16_t Brd_GetMinLimit(uint8_t index);
uint32_t Brd_GetParamStepChange(uint8_t index);




const char* Brd_GetUnit(uint8_t index);
RTC_t Brd_GetRTC();
esp_err_t Brd_SetRTC(RTC_t t);

float Brd_GetPressure();
void Brd_SetPressure(float pressure);

void Brd_PrintAllParameter();
void Brd_LoadDefaultValue();
esp_err_t Brd_WriteParamToFlash();
esp_err_t Brd_ReadParamFromFlash();



/**
 * @brief Cài giá trị integer cho thông số 
 * 
 * @param index thứ tự thông số trong BoardParameter
 * @param val giá trị cần cài đặt
 * @param outputStr chuỗi kết quả sau khi xử lý thành công hoặc lỗi 
 * @return esp_err_t 
 */
esp_err_t Brd_SetParamInt(ParamIndex index,uint32_t val,char *outputStr);
uint32_t Brd_GetParamIntValue(ParamIndex index);

/**
 * @brief Cài đặt các thông số sử dụng giá trị là kiểu string
 * 
 * @param index thứ tự thông số trong BoardParameter
 * @param stringValueIndex thứ tự(index) trong paramValString
 * Ví dụ "Eng" có thứ tự là 3 -> stringValueIndex = 3
 * @param outputStr 
 * @return esp_err_t 
 */
esp_err_t Brd_SetParamStringValueIndex(ParamIndex index,uint8_t *stringValueIndex, char *outputStr);
uint8_t Brd_GetParamStringValueIndex(ParamIndex index);
char* Brd_ConvertStringValueIndexToString(uint8_t stringValueIndex);
#endif
