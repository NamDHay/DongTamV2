#include "MessageHandle.h"
#include "RTC_Format.h"
#include "BoardParameter.h"
#include <stdlib.h>


const char* strRxKey[] = {
	// Receive message
	"DoNothing",
	"SetVan",
	"SetMultiVan",
	"ClearVan",
	"SetPulseTime",
	"SetTotalVan",
	"SetCycleIntervalTime",
	"SetIntervalTime",
	"TrigVan",
	"SetTime",
	"GetTime",
};

const char* strTxKey[] = {
	// Transmit message
	"DoNothing",
	"Van: ",
	"VanState: ",
	"Time: ",
	"Pressure: ",
	"TotalVan: ",
	"PulseTime: ",
	"IntervalTime: ",
	"CycleIntervalTime: ",
	"CycleTime: ",
};

HAL_StatusTypeDef MesgGetValue(MesgValRX mesgValRX, char*inputStr,char *outputStr);

/**
 * @brief Lấy giá trị member trong BoardParameter tương ứng với mesgValTX, đóng gói và trả về chuỗi chứa giá trị member
 * @param mesgValTX thứ tự param trong enum MesgValTX
 * @param outputStr chuỗi trả về để thông báo kết quả xử lý
 * @return HAL_OK nếu xử lý thành công, HAL_ERROR nếu có lỗi xảy ra
 */
HAL_StatusTypeDef MessageTxHandle(MesgValTX mesgValTX,char *outputStr)
{
	char s[40]={0};
	uint16_t len;
	for(uint8_t i = 1; i < (sizeof(strTxKey)/sizeof(char*)); i++){
			// copy key string to send response
			if(i == mesgValTX) {
				strcpy(s,strTxKey[i]);
				break;
			}
	}
	len = strlen(s);
	switch (mesgValTX)
	{
	case TX_VAN:
		itoa(Brd_GetVanOn(),(s+len),2);
		break;
	case TX_VANSTATE:
		sprintf((s+len),"%lu",Brd_GetVanState());
		break;
	case TX_TIME:
		RTC_PackTimeToString(Brd_GetRTC(),(s+len));
		break;
	case TX_PRESSURE:
		sprintf((s+len),"%.2f",Brd_GetPressure());
		break;
	case TX_TOTAL_VAN:
		sprintf((s+len),"%u",Brd_GetTotalVan());
		break;
	case TX_PULSE_TIME:
		sprintf((s+len),"%u",Brd_GetPulseTime());
		break;
	case TX_INTERVAL_TIME:
		sprintf((s+len),"%u",Brd_GetIntervalTime());
		break;
	case TX_CYC_INTV_TIME:
		sprintf((s+len),"%u",Brd_GetCycleIntervalTime());
		break;
	default:
		break;
	}
	strcpy(outputStr,s);
	strcat(outputStr,"\r\n");

	return HAL_OK;
}

/**
 * @brief Xử lý chuỗi nhận được, so sánh với các chuỗi đã có trong strRxKey
 * @param inputStr chuỗi cần xử lý
 * @param outputStr chuỗi trả về để thông báo kết quả xử lý
 * @return HAL_OK nếu xử lý thành công, HAL_ERROR nếu có lỗi xảy ra
 */
HAL_StatusTypeDef MessageRxHandle(char *inputStr, char* outputStr)
{
	uint8_t indexKey = sizeof(strRxKey)/sizeof(char*);
	for(uint8_t i=1;i < indexKey;i++){
		if(strstr(inputStr,strRxKey[i]))
			return MesgGetValue(i,inputStr,outputStr);
		else {
			strcpy(outputStr,"--->No parameter found \n");
			return HAL_ERROR;
		}
	}
	return HAL_OK;
}

/**
 * @brief Tách lấy giá trị từ chuỗi inputStr, gán tương ứng vào các member của BoardParameter
 * @param mesgValRX thứ tự param trong enum MesgValRX
 * @param inputStr chuỗi truyền vào để tách lấy giá trị
 * @param outputStr chuỗi trả về để thông báo kết quả xử lý
 * @return HAL_OK nếu tách giá trị thành công, HAL_ERROR nếu có lỗi xảy ra
 */
HAL_StatusTypeDef MesgGetValue(MesgValRX mesgValRX, char*inputStr,char *outputStr)
{
	RTC_t t;
	uint32_t val;
	uint8_t itemConverted = 0;
	HAL_StatusTypeDef pValRet = HAL_OK;
	if(mesgValRX >= SET_VAN && mesgValRX <= INTERVAL_TIME){
		itemConverted = sscanf(inputStr,MESG_PATTERN_KEY_VALUE_INT,&val);
		// check if value can be obtained from string
		if(itemConverted != 1) {
			strcpy(outputStr,"--->Cannot parse value\n");
			return HAL_ERROR;
		}
	} else if (mesgValRX == SET_TIME){
		t = RTC_GetTimeFromString(inputStr);
		if(Brd_SetRTC(t) == HAL_ERROR) return HAL_ERROR;
	}
	switch(mesgValRX){
		case SET_VAN:
			pValRet = Brd_SetVanOn(val);
			if(pValRet == HAL_OK) MessageTxHandle(TX_VAN,outputStr);
			break;
		case SET_MULTI_VAN:
			pValRet = Brd_SetMultiVan(val);
			if(pValRet == HAL_OK) MessageTxHandle(TX_VAN,outputStr);
			break;
		case CLEAR_VAN:
			pValRet = Brd_SetVanOff(val);
			if(pValRet == HAL_OK) MessageTxHandle(TX_VAN,outputStr);
			break;
		case PULSE_TIME:
			pValRet = Brd_SetPulseTime(val);
			if(pValRet == HAL_OK) MessageTxHandle(TX_PULSE_TIME,outputStr);
			break;
		case TOTAL_VAN:
			pValRet = Brd_SetTotalVan(val);
			if(pValRet == HAL_OK) MessageTxHandle(TX_VAN,outputStr);
			break;
		case CYC_INTV_TIME:
			pValRet = Brd_SetCycleIntervalTime(val);
			if(pValRet == HAL_OK) MessageTxHandle(TX_CYC_INTV_TIME,outputStr);
			break;
		case INTERVAL_TIME:
			pValRet = Brd_SetIntervalTime(val);
			if(pValRet == HAL_OK) MessageTxHandle(TX_INTERVAL_TIME,outputStr);
			break;
		case TRIG_VAN:
			Brd_SetVanProcState(PROC_START);
			break;
		case SET_TIME:
			if(pValRet == HAL_OK) MessageTxHandle(TX_TIME,outputStr);
			return HAL_OK;
			break;
		case GET_TIME:
			if(pValRet == HAL_OK) MessageTxHandle(TX_TIME,outputStr);
			return HAL_OK;
		default:
			return HAL_ERROR;
			break;
	}
	if(pValRet != HAL_OK) {
		strcpy(outputStr,"--->Value beyond limit\n");
		return HAL_ERROR;
	}
	return HAL_OK;
}
