/*
 * MessageHandle.h
 *
 *  Created on: Jun 3, 2023
 *      Author: KHOA
 */

#ifndef INC_MESSAGEHANDLE_H_
#define INC_MESSAGEHANDLE_H_

#include <stdint.h>
#include <string.h>
#include "esp_err.h"

typedef enum MesgValTX{
	TX_SET_VAN = 1,
	TX_SET_MULTIVAN,
	TX_CLEAR_VAN,
	TX_PULSE_TIME,
	TX_TOTAL_VAN,
	TX_CYC_INTV_TIME,
	TX_TRIG_VAN,
	TX_INTERVAL_TIME,// below this is data type that not belong to integer, above this is integer
	TX_SET_TIME,
	TX_GET_TIME,
}MesgValTX;


typedef enum MesgValRX{
	RX_START_INT_VALUE,//mark from this to below will get integer value
	RX_VANSTATE,
	RX_TOTAL_VAN,
	RX_DP_HIGH,
	RX_DP_LOW,
	RX_DP_WARN,
	RX_PULSE_TIME,
	RX_INTERVAL_TIME,
	RX_CYC_INTV_TIME,
	RX_READ_FLASH,
	RX_SAVE_FLASH,
	RX_TRIG_VAN,
	RX_START_FLOAT_VALUE,//mark from this to below will get float value
	RX_PRESSURE,
	RX_START_TIME_FORMAT,//mark from this to below will get time format
	RX_CURRENT_TIME,
	RX_END_MESSAGE,
}MesgValRX;


#define MESG_PATTERN_KEY_VALUE_INT "%*s %lu" //Ex: SetVan: 10
#define MESG_PATTERN_KEY_VALUE_FLOAT "%*s %f" //Ex: Pressure: 0.2543
#define MESG_PATTERN_KEY_VALUE_STRING "%*s \"%s\"" //Ex: SampleString: "Test String"
#define MESG_PATTERN_KEY_VALUE_TIME "%*s %u/%u/%u %u:%u:%u"//Ex: SetTime: 3/6/2023 16:00:00
#define MESG_PATTERN_KEY_INDEX_VALUE_STRING "%*s%u: \"%s\""


typedef esp_err_t (*pValueHandle)(void *pvParameter);

esp_err_t MessageRxHandle(char *inputStr, char* outputStr);
esp_err_t MesgGetValue(MesgValRX mesgValRX, char*inputStr,char *outputStr);
void TX_STM32_TrigVan(uint8_t Trig);
esp_err_t MessageTxHandle(MesgValTX mesgValTX,char *outputStr);


#endif /* INC_MESSAGEHANDLE_H_ */
