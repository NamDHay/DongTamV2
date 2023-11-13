
#include "OnlineStatusEvent.h"
EventGroupHandle_t evgOSE;

void OnlEvt_CreateEventGroup()
{
    evgOSE = xEventGroupCreate();
}

esp_err_t OnlEvt_SetBit(OnlineEventBit bit)
{
    if(!evgOSE) OnlEvt_CreateEventGroup();
    if(bit >= EVT_END) return ESP_ERR_INVALID_ARG;
    xEventGroupSetBits(evgOSE,bit);
    return ESP_OK;
} 

esp_err_t OnlEvt_ClearBit(OnlineEventBit bit)
{
    if(!evgOSE) OnlEvt_CreateEventGroup();
    if(bit >= EVT_END) return ESP_ERR_INVALID_ARG;
    xEventGroupClearBits(evgOSE,bit);
    return ESP_OK;
}

void OnlEvt_WaitBit(OnlineEventBit bit, BaseType_t ClearBit, BaseType_t WaitAll, TickType_t wait)
{
    if(!evgOSE) OnlEvt_CreateEventGroup();
    xEventGroupWaitBits(evgOSE,bit,ClearBit,WaitAll,wait);
}

bool OnlEvt_CheckBit(OnlineEventBit bit)
{
    if(!evgOSE) OnlEvt_CreateEventGroup();
    if((xEventGroupGetBits(evgOSE) & bit) == bit) {
        return 1;
    }
    return 0;
}


