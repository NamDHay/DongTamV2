#ifndef _ONLINE_STATUS_EVENT_H
#define _ONLINE_STATUS_EVENT_H
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
typedef enum{
    EVT_WIFI_CONNECTED,
    EVT_WIFI_FAIL,
    EVT_WIFI_STA_MODE,
    EVT_WIFI_AP_MODE,
    EVT_PING_SUCCESS,
    EVT_PING_TIMEOUT,
    ONL_END_BIT, // not use
}OnlineEventBit;

#define ONL_EVT_WIFI_CONNECTED (1 << EVT_WIFI_CONNECTED)
#define ONL_EVT_WIFI_FAIL (1 << EVT_WIFI_FAIL)
#define ONL_EVT_PING_SUCCESS (1<< EVT_PING_SUCCESS)
#define ONL_EVT_PING_TIMEOUT (1 << EVT_PING_TIMEOUT)
#define ONL_EVT_WIFI_STA (1 << EVT_WIFI_STA_MODE)
#define ONL_EVT_WIFI_AP (1 << EVT_WIFI_AP_MODE)

#define EVT_END (1 << ONL_END_BIT)

esp_err_t OnlEvt_SetBit(OnlineEventBit bit);
esp_err_t OnlEvt_ClearBit(OnlineEventBit bit);
void OnlEvt_WaitBit(OnlineEventBit bit, BaseType_t ClearBit, BaseType_t WaitAll, TickType_t wait);
void OnlEvt_CreateEventGroup();
bool OnlEvt_CheckBit(OnlineEventBit bit);
#endif