#include "JsonHandle.h"

#define MAX_NUM_VAN 16


char* jsHandle_Key[] = {
    "IMEI",
    "Power",
    "FAN",
    "ODCMode",
    "ValveError",
    "VanKich",
    "DeltaPH",
    "DeltaP",
    "DeltaPL",
    "LED10Bar",
    "RTC",
};

cJSON *cJS = NULL;

void jsHandle_Init(char *outputStr)
{
    cJS = cJSON_CreateObject();
    cJSON_AddStringToObject(cJS,jsHandle_Key[JSON_KEY_IMEI],"AC67B2F6E568");
    cJSON_AddNumberToObject(cJS,jsHandle_Key[JSON_KEY_POWER],1);
    cJSON_AddNumberToObject(cJS,jsHandle_Key[JSON_KEY_FAN], 1);
    cJSON_AddNumberToObject(cJS,jsHandle_Key[JSON_KEY_ODC],1);
    cJSON_AddNumberToObject(cJS,jsHandle_Key[JSON_KEY_VAN_ERROR],0);
    cJSON_AddNumberToObject(cJS,jsHandle_Key[JSON_KEY_TRIGGER_VAN],0);
    cJSON_AddNumberToObject(cJS,jsHandle_Key[JSON_KEY_DP_HIGH],3800);
    cJSON_AddNumberToObject(cJS,jsHandle_Key[JSON_KEY_DP],250);
    cJSON_AddNumberToObject(cJS,jsHandle_Key[JSON_KEY_DP_LOW],250);
    cJSON_AddNumberToObject(cJS,jsHandle_Key[JSON_KEY_LED_BAR],5);
    cJSON_AddStringToObject(cJS,jsHandle_Key[JSON_KEY_TIME],"17/8/2023 11:57:00");
    if(outputStr) strcpy(outputStr,cJSON_Print(cJS));
}

char* jsHandle_PrintObject() 
{
    return cJSON_Print(cJS);
}

void jsHandle_DeleteObject()
{
    cJSON_Delete(cJS);
}

cJSON* getItem(JsonKey keyname){
    return cJSON_GetObjectItem(cJS,jsHandle_Key[keyname]);
}

void jsHandle_SetIMEI(char *IMEI){cJSON_SetValuestring(getItem(JSON_KEY_IMEI),IMEI);}
char* jsHandle_GetIMEI(){return cJSON_GetStringValue(getItem(JSON_KEY_IMEI));}

void jsHandle_SetDP(int DP){cJSON_SetIntValue(getItem(JSON_KEY_DP),DP);}
int jsHandle_GetDP(){return cJSON_GetNumberValue(getItem(JSON_KEY_DP));}




void jsHandle_Set_dpLow(int dpLow){if(dpLow < 250 || dpLow > 4000) return;cJSON_SetIntValue(getItem(JSON_KEY_DP_LOW),dpLow);}
int jsHandle_Get_dpLow(){return cJSON_GetNumberValue(getItem(JSON_KEY_DP_LOW));}

void jsHandle_Set_dpHigh(int dpHigh){if(dpHigh < 250 || dpHigh > 4000) return; cJSON_SetIntValue(getItem(JSON_KEY_DP_HIGH),dpHigh);}
int jsHandle_Get_dpHigh() {return cJSON_GetNumberValue(getItem(JSON_KEY_DP_HIGH));}

void jsHandle_Set_dpIndicatorBar(uint8_t level){if(level > 10) return;cJSON_SetIntValue(getItem(JSON_KEY_LED_BAR),(int)level);}
int jsHandle_Get_dpIndicatorBar(){return cJSON_GetNumberValue(getItem(JSON_KEY_LED_BAR));}


// void UpdateGetTime(cJSON *item,char *logMessage)
// {
//     if(!cJSON_IsString(item)) return;
//     char T[20];
//     strcpy(T,cJSON_GetStringValue(item));
//     brdParam.Time = RTC_GetTimeFromString(T);
//     RTC_PackTimeToString(brdParam.Time,T);
//     sprintf(logMessage,"GetTime update:\t%s\n",T);
// }

// void UpdateSetTime(cJSON *item,char *logMessage)
// {
//     if(!cJSON_IsString(item)) return;
//     char T[20];
//     strcpy(T,cJSON_GetStringValue(item));
//     brdParam.Time = RTC_GetTimeFromString(T);
//     RTC_PackTimeToString(brdParam.Time,T);
//     sprintf(logMessage,"SetTime update:\t%s\n",T);
//     xEventGroupSetBits(evg1,EVT_SET_TIME);
// }

// void UpdateTrigVan(cJSON *item,char *logMessage)
// {
//     sprintf(logMessage,"TrigVan is send\n");
//     xEventGroupSetBits(evg1,EVT_TRIG_VAN);
// }

// void UpdatePressure(cJSON *item,char *logMessage)
// {
//     if(!cJSON_IsNumber(item)) return;
//     brdParam.Pressure = cJSON_GetNumberValue(item);
//     sprintf(logMessage,"Pressure update: %.4f",brdParam.Pressure);
// }

// void UpdateSetVan(cJSON *item,char *logMessage)
// {
//     if(!cJSON_IsNumber(item)) return;
//     uint32_t a = (uint32_t) cJSON_GetNumberValue(item);
//     if(a > MAX_NUM_VAN - 1) {
//         sprintf(logMessage,"Input van > MAX_NUM_VAN:%d",MAX_NUM_VAN);
//         return;
//     }
//     brdParam.SetVan = a;
//     sprintf(logMessage,"SetVan update:\t%ld\n",brdParam.SetVan);
//     xEventGroupSetBits(evg1,EVT_SET_VAN);
// }

// void UpdateClearVan(cJSON *item,char *logMessage)
// {
//     if(!cJSON_IsNumber(item)) return;
//     uint32_t a = (uint32_t) cJSON_GetNumberValue(item);
//     if(a > MAX_NUM_VAN - 1) {
//         sprintf(logMessage,"Input van > MAX_NUM_VAN:%d",MAX_NUM_VAN);
//         return;
//     }
//     brdParam.ClearVan = a;
//     sprintf(logMessage,"ClearVan update:\t%ld\n",brdParam.ClearVan);
//     xEventGroupSetBits(evg1,EVT_CLEAR_VAN);
// }

// void UpdateVanValue(cJSON *item,char *logMessage)
// {
//     if(!cJSON_IsNumber(item)) return;
//     brdParam.VanData = cJSON_GetNumberValue(item);
//     sprintf(logMessage,"VanValue update:\t%ld\n",brdParam.VanData);
// }

// void UpdateVanState(cJSON *item,char *logMessage)
// {
//     if(!cJSON_IsString(item)) return;
//     strcpy(brdParam.VanState,cJSON_GetStringValue(item));
//     sprintf(logMessage,"VanState update:\t%s\n",brdParam.VanState);
// }

// void SendPrintParamEvt(cJSON *item,char *logMessage)
// {
//     xEventGroupSetBits(evg1,EVT_GET_FULL_PARAM);
//     strcpy(logMessage,"Trig GET_FUL_PARAM evt");
// }

// void ObtainJsonItem(cJSON *cjs, char *JsonKey,void (*pParamUpdate)(cJSON *item,char *logMessage))
// {
//     char s[35];
//     if(cJSON_HasObjectItem(cjs,JsonKey)){
//         cJSON *item = cJSON_GetObjectItemCaseSensitive(cjs,JsonKey);
//         pParamUpdate(item,s);
//         SendStringToUART(qLogTx,s);
//         cJSON_DeleteItemFromObject(cjs,JsonKey); 
//     }
// }



// void UpdateParamFromParsedJsonItem(cJSON *cjs)
// {
//     // Section: Parse Item to get value from STM32
//     ObtainJsonItem(cjs,JSON_PARSE_KEY_PRESSURE,&UpdatePressure);
//     ObtainJsonItem(cjs,JSON_PARSE_KEY_TIME,&UpdateGetTime);
//     ObtainJsonItem(cjs,JSON_PARSE_KEY_VAN_VALUE,&UpdateVanValue);
//     ObtainJsonItem(cjs,JSON_PARSE_KEY_VANSTATE,&UpdateVanState);
//     // End section
//     // Section: Parse item to set value to STM32
//     ObtainJsonItem(cjs,JSON_PARSE_KEY_GET_ALL_PARAM,&SendPrintParamEvt);
//     ObtainJsonItem(cjs,JSON_PARSE_KEY_SET_VAN,&UpdateSetVan);
//     ObtainJsonItem(cjs,JSON_PARSE_KEY_CLEAR_VAN,&UpdateClearVan);
//     ObtainJsonItem(cjs,JSON_PARSE_KEY_TRIG_VAN,&UpdateTrigVan);
//     ObtainJsonItem(cjs,JSON_PARSE_KEY_SET_TIME,&UpdateSetTime);
//     // End section
// }

// char* PrintJsonGetItem(cJSON *object, char *jsonKey)
// {
//     if(!cJSON_HasObjectItem(object,jsonKey)) cJSON_AddStringToObject(object,jsonKey,"");   
//     else cJSON_SetValuestring(cJSON_GetObjectItem(object,jsonKey),""); 
//     return cJSON_Print(object);
// }

// char* PrintJsonSetTime(cJSON *object, char *jsonKey)
// {
//     char s[20];
//     RTC_PackTimeToString(brdParam.Time,s);
//     if(!cJSON_HasObjectItem(object,jsonKey)) cJSON_AddStringToObject(object,jsonKey,s);   
//     else cJSON_SetValuestring(cJSON_GetObjectItem(object,jsonKey),s);
//     return cJSON_Print(object);
// }

// char* PrintJsonSetVan(cJSON *object, char *jsonKey)
// {
//     if(!cJSON_HasObjectItem(object,jsonKey)) cJSON_AddNumberToObject(object,jsonKey,brdParam.SetVan);   
//     else cJSON_SetIntValue(cJSON_GetObjectItem(object,jsonKey),brdParam.SetVan);
//     return cJSON_Print(object);
// }

// char* PrintJsonClearVan(cJSON *object, char *jsonKey)
// {
//     if(!cJSON_HasObjectItem(object,jsonKey)) cJSON_AddNumberToObject(object,jsonKey,brdParam.ClearVan);   
//     else cJSON_SetIntValue(cJSON_GetObjectItem(object,jsonKey),brdParam.ClearVan);
//     return cJSON_Print(object);
// }

// char* PrintJsonTrigVan(cJSON *object, char *jsonKey)
// {
//     if(!cJSON_HasObjectItem(object,jsonKey)) cJSON_AddTrueToObject(object,jsonKey);  
//     return cJSON_Print(object);
// }

// char* PrintJsonGetPressure(cJSON *object, char *jsonKey)
// {
//     if(!cJSON_HasObjectItem(object,jsonKey)) cJSON_AddNumberToObject(object,jsonKey,brdParam.Pressure);   
//     else cJSON_SetNumberValue(cJSON_GetObjectItem(object,jsonKey),brdParam.Pressure);
//     return cJSON_Print(object);
// }



// void CheckEvent_PackJsonData_Send( cJSON *cjs, 
//                                 char *jsonKey,
//                                 EventBits_t e, 
//                                 uint32_t event, 
//                                 char* (*pJsPrint)(cJSON * object, char *jsonKey))
// {
//     if(CHECKFLAG(e,event)){
//         SendStringToUART(qSTM32Tx,pJsPrint(cjs,jsonKey));
//         cJSON_DeleteItemFromObjectCaseSensitive(cjs,jsonKey);
//     }
// }



