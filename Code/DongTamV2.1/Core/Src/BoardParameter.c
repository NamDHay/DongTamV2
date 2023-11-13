/*
 * BoardParameter.c
 *
 *  Created on: Jun 6, 2023
 *      Author: KHOA
 */


#include "BoardParameter.h"
#include "MessageHandle.h"
#include "main.h"

#include <stdlib.h>

BoardParameter brdParam;
extern UART_HandleTypeDef huart3;

VanProcedure vanProcState;

uint16_t CheckVanInUsed(uint16_t *currentVanOn);

/**
 * @brief Thực hiện kích đóng van
 * @param outputStr Gửi giá trị áp suất sau khi thực hiện xong
 * @param VanTrigger thứ tự van cần kích
 */
void VanOn(char *outputStr,uint8_t VanTrigger)
{
	if(VanTrigger > 16) return;
	HC595_SetBitOutput(VanTrigger);
	HC595_ShiftOut(NULL, 2, 1);
	MessageTxHandle(TX_PRESSURE, outputStr);
	vanProcState = BRD_PULSE_TIME;

}

/**
 * @brief Tắt van (hở mạch, không có dòng qua cuộn dây của van)
 * @param outputStr Gửi giá trị trạng thái van thông báo sự cố van nếu có
 * @param VanTrigger thứ tự van cần tắt
 */
void VanOff(char *outputStr,uint8_t VanTrigger)
{
	if(VanTrigger > 16) return;
	MessageTxHandle(TX_VANSTATE, outputStr);
	HC595_ClearBitOutput(VanTrigger);
	HC595_ShiftOut(NULL, 2, 1);
	vanProcState = BRD_INTERVAL_TIME;
}

/**
 * @brief Kiểm tra số chu kỳ còn lại trong chu trình kích van
 * @param cycleTime số chu kỳ còn lại, nếu chu kỳ còn lại = 0 thì chuyển sang kết thúc chu trình
 * @param currentVanOn khi kích hết van, reset lại số van cần kích như ban đầu để bắt đầu chu trình kích van mới
 */
void CheckCycleIntervalTime(uint16_t *cycleTime,uint16_t *currentVanOn)
{
	(*cycleTime) -= 1;
	if(*cycleTime > 0) {
		*currentVanOn = Brd_GetVanOn();
		Brd_SetVanProcState(BRD_VAN_ON);
	}
	else {
		vanProcState = PROC_END;
	}
}

/**
 * @brief Cài đặt thời gian kích van (kín mạch cho phép dòng điện chạy qua cuộn dây)
 * @param outputStr Gửi liên tục giá trị áp suất đọc được
 */
void PulseTimeHandle(char *outputStr)
{
	MessageTxHandle(TX_PRESSURE, outputStr);
	if(Brd_GetTimerArray(0) * TIMER_PERIOD_MS >= Brd_GetPulseTime()){
		Brd_SetTimerArray(0,0);
		Brd_SetVanProcState(BRD_VAN_OFF);
	}
}

/**
 * @brief
 * @param currentVanOn Lấy ra vị trí van cần kích và load vị trí van tiếp theo
 * @return Trả về thứ tự van đang được kích
 */
uint16_t CheckVanInUsed(uint16_t *currentVanOn)
{
	if(*currentVanOn > 0){
		for(uint8_t i=0;i<MAX_NUM_VAN;i++){
			if((*currentVanOn & (1<<i)) != 0) {
				char s[16]={0};
				itoa(*currentVanOn,s,2);
				*currentVanOn &=~ (1<<i);
				return i;
			}
		}
	}
	return HAL_ERROR;
}

/**
 * @brief Thời gian nghỉ giữa 2 lần kích van, nếu vẫn còn van kích thì chờ đủ thời gian rồi kích van tiếp theo
 * @param currentVanOn nếu không còn van nào được kích thì chuyển sang trạng thái kiểm tra số chu kỳ còn lại
 */
void IntervalTimeHandle(uint16_t *currentVanOn)
{
	//if interval time is passed and no van to trigger
	if(Brd_GetTimerArray(1)*TIMER_PERIOD_MS >= Brd_GetIntervalTime()*1000 && !*currentVanOn){
		// reset interval time
		Brd_SetTimerArray(1, 0);
		// set it to check cycle interval time
		Brd_SetVanProcState(BRD_CYCLE_INTERVAL_TIME);
	} else if(Brd_GetTimerArray(1)*TIMER_PERIOD_MS >= Brd_GetIntervalTime()*1000){
		// reset interval time
		Brd_SetTimerArray(1, 0);
		Brd_SetVanProcState(BRD_VAN_ON);
	}
}

/**
 * Thực thi chu trình kích van
 * @param outputStr chuỗi trả về tương ứng với các giai đoạn khác nhau trong chu trình
 */
void ProcedureTriggerVan(char *outputStr)
{
	static uint16_t cycleTime;
	static uint16_t VanToTrigger;
	static uint16_t currentVanOn;
	switch(vanProcState){
		case PROC_IDLE:
			break;
		case PROC_START:
			cycleTime = Brd_GetCycleIntervalTime();
			currentVanOn = Brd_GetVanOn();
			vanProcState = BRD_VAN_ON;
			break;
		case BRD_VAN_ON:
			VanToTrigger = CheckVanInUsed(&currentVanOn);
			LogDataValue("VanToTrig:", VanToTrigger);
			if(VanToTrigger > 16){
				vanProcState = BRD_CYCLE_INTERVAL_TIME;
				break;
			}
			VanOn(outputStr,VanToTrigger);
			break;
		case BRD_VAN_OFF:
			VanOff(outputStr,VanToTrigger);
			if(!Brd_GetHC165State()) Brd_SetHC165State(true);
			break;
		case BRD_PULSE_TIME:
			PulseTimeHandle(outputStr);
			break;
		case BRD_INTERVAL_TIME:
			IntervalTimeHandle(&currentVanOn);
			break;
		case BRD_CYCLE_INTERVAL_TIME:
			CheckCycleIntervalTime(&cycleTime,&currentVanOn);
			LogDataValue("CycleInterval:", cycleTime);
			break;
		case PROC_END:
			vanProcState = PROC_IDLE;
		break;
	}
}

uint32_t Brd_GetVanState(){return HC165_ReadState(2);}


uint16_t Brd_GetTotalVan(){return brdParam.totalVan;}
uint16_t Brd_GetVanOn(){return brdParam.currentVanOn;}
uint16_t Brd_GetIntervalTime(){return brdParam.intervalTime;}
uint16_t Brd_GetPulseTime(){return brdParam.pulseTime;}
uint16_t Brd_GetCycleIntervalTime(){return brdParam.cycIntvTime;}
RTC_t Brd_GetRTC()
{
	brdParam.RTCtime = PCF8563_ReadTimeRegisters();
	return brdParam.RTCtime;
}

float Brd_GetPressure()
{
	brdParam.pressure = AMS5915_CalPressure(Brd_GetAddress_AMS5915());
	return brdParam.pressure;
}

HAL_StatusTypeDef Brd_SetTotalVan(uint8_t val)
{

    if(val > 0 && val <= 16){
        brdParam.totalVan = val;
        // reset all current valve
        brdParam.currentVanOn = 0;
        // fill all output bit from zero to total van
        for(uint8_t i = 0; i < val; i++){
        	brdParam.currentVanOn |=  (1 << i);
        }
        return HAL_OK;
    }
    return HAL_ERROR;
}

HAL_StatusTypeDef Brd_SetMultiVan(uint16_t val)
{
	if(val > 65535) return HAL_ERROR;
	brdParam.currentVanOn =  val;
    return HAL_OK;
}

HAL_StatusTypeDef Brd_SetVanOn(uint16_t val)
{

    if(val < 16){
        brdParam.currentVanOn |=  (1 << (val));
    }
    else return HAL_ERROR;
    return HAL_OK;
}

HAL_StatusTypeDef Brd_SetVanOff(uint16_t val)
{
    if(val < 16){
    	brdParam.currentVanOn &=  ~(1 << (val));
    	return HAL_OK;
    }
    else return HAL_ERROR;
}

HAL_StatusTypeDef Brd_SetIntervalTime(uint16_t val)
{
    if(val > 0 && val <= 100){
        brdParam.intervalTime = val;
		return HAL_OK;
    }
    return HAL_ERROR;
}

HAL_StatusTypeDef Brd_SetPulseTime(uint16_t val)
{
    if(val >= 30 && val <= 300) {
        brdParam.pulseTime = val;
        return HAL_OK;
    }
    return HAL_ERROR;
}

HAL_StatusTypeDef Brd_SetRTC(RTC_t t){
    uint8_t indexT = sizeof(t) / (sizeof(int8_t));
    int8_t *a = &t.year;
    for(uint8_t i = 0; i < indexT; i++){
        if(*(a+i) == -1) return HAL_ERROR;
    }
    brdParam.RTCtime = t;
    PCF8563_WriteTimeRegisters(brdParam.RTCtime);
    return HAL_OK;
}



HAL_StatusTypeDef Brd_SetCycleIntervalTime(uint16_t val)
{
    if(val > 2 && val <= 100){
        brdParam.cycIntvTime = val;
    	return HAL_OK;
    }
	return HAL_ERROR;
}
uint16_t Brd_GetTimerArray(uint8_t element){return brdParam.timerArray[element];}

HAL_StatusTypeDef Brd_SetTimerArray(uint8_t element, uint16_t val)
{
	if(element >= sizeof(brdParam.timerArray)) return HAL_ERROR;
	brdParam.timerArray[element] = val;
	return HAL_OK;
}

void Brd_SetHC165State(bool state){brdParam.HC165_state = state;}

VanProcedure Brd_GetVanProcState(){return vanProcState;}
void Brd_SetVanProcState(VanProcedure state){vanProcState = state;}
HC595* Brd_GetAddress_HC595(){return &brdParam.hc595;}
HC165* Brd_GetAddress_HC165(){return &brdParam.hc165;}
PCF8563_Handle* Brd_GetAddress_PCF8563(){return &brdParam.pcf;}
AMS5915* Brd_GetAddress_AMS5915(){return &brdParam.ams;}
bool Brd_GetHC165State(void){return brdParam.HC165_state;}


int8_t LogDataValue(char *s,uint32_t value)
{
	if(strlen(s) > 30) {
		HAL_UART_Transmit(&huart3, (uint8_t*)"Oversize\n", strlen("Oversize\n"), HAL_MAX_DELAY);
		return HAL_ERROR;
	}
	char sTemp[30]={0};
	uint16_t len = strlen(s);
	strcpy(sTemp,s);
	sprintf((sTemp+len),"%lu\n",value);
	HAL_UART_Transmit(&huart3, (uint8_t*)sTemp, strlen(sTemp), HAL_MAX_DELAY);
	return HAL_OK;
}


