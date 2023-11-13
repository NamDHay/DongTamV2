#ifndef SHARE_VAR_H
#define SHARE_VAR_H

#include "LCD_I2C.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "cJSON.h"
#include "RTC_Format.h"
#include "GUI/GUI.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "nvs.h"

#define USE_SCROLL_SCREEN 

#define EVT_UART_TASK_BIG_SIZE (1<<0)
#define EVT_UART_DELETE_TASK_BIG_SIZE (1<<1)
#define EVT_UART_OVERSIZE_HW_FIFO (1<<2)
#define EVT_UART_STM32_READY (1<<3)
extern LCDI2C lcdI2C;
extern QueueHandle_t qUartHandle,qSTM32Tx,qLogTx;
extern uart_port_t uartTarget;
extern EventGroupHandle_t evgGUI;
extern EventGroupHandle_t evgUART;
#endif