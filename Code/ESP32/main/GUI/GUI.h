#ifndef __GUI_H__
#define __GUI_H__

#include "esp_err.h"
#include "i2cdev.h"
#include "LCD_I2C.h"
#include "esp_check.h"
#include "PressureIndicator.h"
#include "74HC595.h"
#include "GUI.h"

#include "../BoardParameter.h"
#include "../main.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define LED_ERROR_MASK 10
#define LED_STATUS_MASK 11

#define EVT_BTN_MENU (1<<0)
#define EVT_BTN_SET (1<<1)
#define EVT_BTN_UP (1<<2)
#define EVT_BTN_DOWN_RIGHT (1<<3)

#define EVT_SET_VALUE_TO_FLASH (1<<4)
#define EVT_GET_VALUE_FROM_FLASH (1<<5)
#define EVT_VALUE_ABOVE_THRESHOLD (1<<6)
#define EVT_VALUE_BELOW_THRESHOLD (1<<7)
#define EVT_INCREASE_VALUE (1<<8)
#define EVT_DECREASE_VALUE (1<<9)
#define EVT_PARAM_SCROLL_UP (1<<10)
#define EVT_PARAM_SCROLL_DOWN (1<<11)

#define EVT_LCD_RESET (1 << 12)

#define LENGTH_OF_PARAM     11 //length of paramText

typedef struct {
    void* Value;    
    uint16_t scaleValue; // multiply with GUI_NAV.value to get actual result
    uint8_t index;
}GUIParam_t;

#define LCD_COLS 20
#define LCD_ROWS 4

void LedErrorWrite(bool ledState);
void LedStatusWrite(bool ledState);
void ReadGuiButton(gpio_num_t gpio, EventBits_t e);
void GUITask(void *pvParameter);
void TaskScanButton(void *pvParameter);
void GuiInit();
void GuiTestFull();
void GUI_ClearPointer();
void GUI_ShowPointer();
void GUI_LoadPageAtInit();
void GUI_GetParam(GUIParam_t *gp, ParamIndex paramNO);
// void GUI_SetGuiInfoValue(GUI_Info *gi, uint8_t paramNO, uint32_t value);
void GUI_LoadPage();
void GUI_resetLCD();
TaskHandle_t* GUI_GetTaskHandle();
#endif