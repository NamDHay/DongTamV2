#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "HTTP_POSTGET.h"
#include "WiFi.h"
#include "DNS_Ping.h"
#include "OnlineStatusEvent.h"
#include "RTC_Format.h"
#include "nvs.h"
#include "nvs_flash.h"
TimerHandle_t tOnl,tRTC; 
TaskHandle_t taskHandleOnl;

RTC_t rtc;
nvs_handle_t nvsReset;

#define TIMER_COUNT_EXPIRE_TRIGGER_RECONNECT 5
#define TIMER_COUNT_EXPIRE_TRIGGER_PING 10
TaskHandle_t *TaskOnl_GetHandle() {return &taskHandleOnl;}

void TimerOnline_Callback(TimerHandle_t xTimer)
{   
    uint32_t count = (uint32_t)pvTimerGetTimerID(xTimer);
    count ++; 
    vTimerSetTimerID(xTimer,(void*)count);
}

void onl_Handle_Reconnect_Sequence(){
    static bool IsTimerStop = 1; // set to 1 because it is stop at the initialize
    uint32_t countTimer = 0;
    if(OnlEvt_CheckBit(ONL_EVT_PING_SUCCESS | ONL_EVT_WIFI_CONNECTED)) {
        if(!IsTimerStop){
            xTimerStop(tOnl,10/portTICK_PERIOD_MS);
            IsTimerStop = 1;
        }
        return;
    }
    if(IsTimerStop){
        xTimerStart(tOnl,10/portTICK_PERIOD_MS);
        IsTimerStop = 0;
    }
    if(OnlEvt_CheckBit(ONL_EVT_WIFI_FAIL)){
        countTimer = (uint32_t)pvTimerGetTimerID(tOnl);
        ESP_LOGI("Reconnect","timer count:%lu",countTimer);
        vTaskDelay(3000/portTICK_PERIOD_MS);
        if(countTimer >= TIMER_COUNT_EXPIRE_TRIGGER_RECONNECT){
            vTimerSetTimerID(tOnl,0); // reset counter
            esp_wifi_connect();
        }
    }
    else if(OnlEvt_CheckBit(ONL_EVT_PING_TIMEOUT) || OnlEvt_CheckBit(ONL_EVT_WIFI_CONNECTED)){
        if(!DNS_GetCurrentHost()) {
            if(DNS_PingToHost(DNS_GOOGLE) != ESP_OK) {
                vTaskDelay(3000/portTICK_PERIOD_MS);
                ESP_LOGE("Network","No internet");
                return;
            }
        }
        countTimer = (uint32_t)pvTimerGetTimerID(tOnl);
        vTaskDelay(5000/portTICK_PERIOD_MS);
        if(countTimer >= TIMER_COUNT_EXPIRE_TRIGGER_PING){
            ESP_LOGI("CheckDNS","Pinging");
            vTimerSetTimerID(tOnl,0);
            if(!DNS_IsPinging()) DNS_StartToPing();
        }
    }
}

esp_err_t readResetTime(uint8_t *resetTime)
{
    esp_err_t err;
    err = nvs_open("Board", NVS_READONLY, &nvsReset);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } 
    else {
        size_t sz;
        err = nvs_get_blob(nvsReset,"RstT",NULL,&sz);
        if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
        err = nvs_get_blob(nvsReset,"RstT",resetTime,&sz);
        if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    }
    nvs_close(nvsReset);
	ESP_LOGI("BoardReadFlash","Read success");
    return err;

}

esp_err_t writeResetTime(uint8_t *resetTime)
{
	esp_err_t err;
	err = nvs_open("Board", NVS_READWRITE, &nvsReset);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } 
    else {
        err = nvs_set_blob(nvsReset,"RstT",(void*)resetTime,sizeof(resetTime));
        err = nvs_commit(nvsReset);
    }
    nvs_close(nvsReset);
    ESP_LOGI("BoardWriteFlash","Write parameter and close");
    vTaskDelay(10/portTICK_PERIOD_MS);
	return err;
}

void TaskOnlManage(void *pvParameter)
{
    uint8_t resetTime = 0;
    if(readResetTime(&resetTime) != ESP_OK){
        if(writeResetTime(&resetTime) == ESP_OK)
        ESP_LOGW("TaskOnl","Not found reset time in flash, write it in flash for the first time");
    } else {
        resetTime ++;
        if(writeResetTime(&resetTime) == ESP_OK) 
        ESP_LOGW("TaskOnl","resetTime:%u",resetTime);
    }


    ESP_LOGI("TaskOnl","Succesfully created");
    tOnl = xTimerCreate("TimerOnlineManage",pdMS_TO_TICKS(1000),pdTRUE,0,TimerOnline_Callback);
    xTimerStop(tOnl,portMAX_DELAY);
    wifi_init_sta();
    while (1){
        onl_Handle_Reconnect_Sequence();
        onl_HTTP_SendToServer((int)resetTime);
        printf("%lu\n",esp_get_free_heap_size());
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
    
}