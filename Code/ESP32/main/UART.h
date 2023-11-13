#ifndef MAIN_UART_H_
#define MAIN_UART_H_
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

#include "./ShareVar.h"


#define UART_RX GPIO_NUM_16
#define UART_TX GPIO_NUM_17
#define RD_BUF_SIZE 10
#define UART_QUEUE_EVENT_SIZE 34
#define UART_BUFFER_EVENT_SIZE 1000



#define QUEUE_RX qUartHandle

void TaskUart(void *pvParameters);
void UARTConfig();
#endif
