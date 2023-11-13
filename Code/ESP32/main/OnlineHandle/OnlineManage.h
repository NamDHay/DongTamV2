#ifndef ONL_MANAGE_H
#define ONL_MANAGE_H
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "WiFi.h"
#include "DNS_Ping.h"
#include "OnlineStatusEvent.h"
void TaskOnlManage(void *pvParameter);
TaskHandle_t *TaskOnl_GetHandle();
#endif
