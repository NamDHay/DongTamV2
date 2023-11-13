#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_event.h"
#include "OnlineStatusEvent.h"
#include "JsonHandle/JsonHandle.h"
#include "BoardParameter.h"
#include "POSTGET.h"
#include "HTTP_POSTGET.h"
#include "GUI/PressureIndicator.h"
HTTP_CODE_e onl_HTTP_SendToServer(int a)
{//%d/%d/%d %d:%d:%d
    uint32_t value;
    HTTP_CODE_e http_code = HTTP_INVALID;
    if(!OnlEvt_CheckBit(ONL_EVT_WIFI_CONNECTED) || !OnlEvt_CheckBit(ONL_EVT_PING_SUCCESS)) return 0;
    if(xTaskNotifyWait(pdFALSE,pdTRUE,&value,10/portTICK_PERIOD_MS)){
        RTC_t t = Brd_GetRTC();
        char s[300] = {0};
        snprintf(s,300,  
        "{'IMEI': \"AC67B2F6E568\", 'Power':1, 'FAN':1, 'ODCMode':1, 'ValveError':0, 'VanKich':0, 'DeltaPH':%ld, 'DeltaP':%.2f,'DeltaPL':%ld,'LED10Bar':%u,'RTC': \"%d/%d/%d %d:%d:%d\"}",
        Brd_GetParamIntValue(INDEX_DP_HIGH),
        Brd_GetPressure(),
        Brd_GetParamIntValue(INDEX_DP_LOW),
        PI_CalcLevelFromPressure(Brd_GetPressure()),
        t.day,t.month,t.year+2000,t.hour,t.minute,t.second);
        http_code = http_post(URL_POST_IOTVISION_DONGTAM,s);
    }
    return http_code;
}




