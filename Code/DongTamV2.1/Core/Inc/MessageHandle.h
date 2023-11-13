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
#include "main.h"

typedef enum MesgValRX{
	SET_VAN = 1,
	SET_MULTI_VAN,
	CLEAR_VAN,
	PULSE_TIME,
	TOTAL_VAN,
	CYC_INTV_TIME,
	INTERVAL_TIME,
	TRIG_VAN,
	SET_TIME,
	GET_TIME,
}MesgValRX;


typedef enum MesgValTX{
	TX_VAN = 1,
	TX_VANSTATE,
	TX_TIME,
	TX_PRESSURE,
	TX_TOTAL_VAN,
	TX_PULSE_TIME,
	TX_INTERVAL_TIME,
	TX_CYC_INTV_TIME,
}MesgValTX;


#define MESG_PATTERN_KEY_VALUE_INT "%*s %lu" //Ex: SetVan: 10
#define MESG_PATTERN_KEY_VALUE_FLOAT "%*s %f" //Ex: Pressure: 0.2543
#define MESG_PATTERN_KEY_VALUE_STRING "%*s \"%s\"" //Ex: SampleString: "Test String"
#define MESG_PATTERN_KEY_VALUE_TIME "%*s %u/%u/%u %u:%u:%u"//Ex: SetTime: 3/6/23 16:00:00
#define MESG_PATTERN_KEY_INDEX_VALUE_STRING "%*s%u: \"%s\""


typedef HAL_StatusTypeDef (*pValueHandle)(void *pvParameter);

HAL_StatusTypeDef MessageRxHandle(char *inputStr, char* outputStr);
HAL_StatusTypeDef MessageTxHandle(MesgValTX mesgValTX,char *outputStr);


#endif /* INC_MESSAGEHANDLE_H_ */
