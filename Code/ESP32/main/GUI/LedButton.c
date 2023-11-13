#include "74HC595.h"
#include "driver/gpio.h"
#include "LedButton.h"
#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "GUI_Navigation.h"
#include "GUI.h"

void TaskScanButton(void *pvParameter){
    while(1){
        ReadGuiButton(BTN_MENU,EVT_BTN_MENU);
        ReadGuiButton(BTN_UP,EVT_BTN_UP);
        ReadGuiButton(BTN_DOWN_RIGHT,EVT_BTN_DOWN_RIGHT);
        ReadGuiButton(BTN_SET,EVT_BTN_SET);
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

void LedErrorWrite(bool ledState)
{
    if(ledState) HC595_SetBitOutput(LED_ERROR_MASK);
    else HC595_ClearBitOutput(LED_ERROR_MASK);
}

void LedStatusWrite(bool ledState)
{
    if(ledState) HC595_SetBitOutput(LED_STATUS_MASK);
    else HC595_ClearBitOutput(LED_STATUS_MASK);
}

void BtnHandleWhenHolding(gpio_num_t gpio, EventBits_t e){

#define BTN_HOLD_DELAY_MAX 300
#define BTN_HOLD_DELAY_MIN 50
#define BTN_HOLD_DELAY_DECREASE_STEP 50
#define DELAY_COUNT_LOOP_THRESHOLD 3
#define RESET_DELAY 2
    //Delay each loop when user keep holding button
    static uint16_t Delay = BTN_HOLD_DELAY_MAX;
    /* if this number is bigger than certain threshold, 
    Delay will be decrease to speed up sending notify to GUITask
    Each time call xTaskNotify, DelayCountLoop increase by 1
    */
    static uint16_t DelayCountLoop = 0;
    static uint8_t LCD_ResetCount = 0;
    if(e == RESET_DELAY){
        DelayCountLoop = 0;
        LCD_ResetCount = 0;
        Delay = BTN_HOLD_DELAY_MAX;
        // do nothing after reset, so must be return
        return;
    }


    /*if user keep holding down button, check button is UP or DOWN-RIGHT and if only currently selected is value 
    to make delay shorter, the rest is just delay
    
    */
    if(((gpio == BTN_UP) || (gpio == BTN_DOWN_RIGHT)) 
    && (GUINAV_GetCurrentSelected() == IS_VALUE)) {
        TaskHandle_t *taskGUIHandle = GUI_GetTaskHandle();
        vTaskDelay(Delay/portTICK_PERIOD_MS);
        DelayCountLoop+=1;
        /*
        if user only press BTN_UP and BTN_DOWN_RIGHT just only one, interpret it as normal button and not send notify, if user holding down button and DelayCountLoop increase, this time interpret it as speed up count
        */
        if(DelayCountLoop > DELAY_COUNT_LOOP_THRESHOLD - 1) 
        xTaskNotify(*taskGUIHandle,e,eSetValueWithoutOverwrite);
    }
    // now button MENU should be handle to reset LCD if it is error
    else if(gpio == BTN_MENU){
        ESP_LOGI("Holding Menu","%u",LCD_ResetCount);
        vTaskDelay(1000/portTICK_PERIOD_MS);
        LCD_ResetCount++;
        // Wait for 5 seconds or above to send notify 
        if(LCD_ResetCount >= 5){
            LCD_ResetCount = 0;
            TaskHandle_t *taskGUIHandle = GUI_GetTaskHandle();
            xTaskNotify(*taskGUIHandle,EVT_LCD_RESET,eSetValueWithoutOverwrite);
        }
    }
    else {
        // button SET are nothing to hanle, just delay and return
        vTaskDelay(100/portTICK_PERIOD_MS);
        return;
    }


    if(DelayCountLoop >= DELAY_COUNT_LOOP_THRESHOLD && Delay > BTN_HOLD_DELAY_MIN){
        DelayCountLoop=0;
        int temp = Delay - BTN_HOLD_DELAY_DECREASE_STEP;
        // Saturate low delay speed
        if(temp < BTN_HOLD_DELAY_MIN) Delay = BTN_HOLD_DELAY_MIN;
        else Delay-=BTN_HOLD_DELAY_DECREASE_STEP; 
    }
    
}

void ReadGuiButton(gpio_num_t gpio, EventBits_t e)
{
    if(!gpio_get_level(gpio)){
        vTaskDelay(100/portTICK_PERIOD_MS);
        while (!gpio_get_level(gpio)){
            // do things while holding button, i.e count UP and DOWN continously
            BtnHandleWhenHolding(gpio,e);
        }
        TaskHandle_t *taskGUIHandle = GUI_GetTaskHandle(); 
        xTaskNotify(*taskGUIHandle,e,eSetValueWithoutOverwrite);
        // Send RESET_DELAY to reset Delay and DelayCountLoop
        BtnHandleWhenHolding(0,2);
    }
}

void TestLedStatusErr(uint8_t blinkNum,uint16_t delay)
{
    for(uint8_t i=0;i<blinkNum;i++){
        LedErrorWrite(1);
        HC595_ShiftOut(NULL,2,1);
        vTaskDelay(delay/portTICK_PERIOD_MS);
        LedErrorWrite(0);
        HC595_ShiftOut(NULL,2,1);
        vTaskDelay(delay/portTICK_PERIOD_MS);
    }
    for(uint8_t i=0;i<blinkNum;i++){
        LedStatusWrite(1);
        HC595_ShiftOut(NULL,2,1);
        vTaskDelay(delay/portTICK_PERIOD_MS);
        LedStatusWrite(0);
        HC595_ShiftOut(NULL,2,1);
        vTaskDelay(delay/portTICK_PERIOD_MS);
        
    }
    for(uint8_t i=0;i<blinkNum;i++){
        LedStatusWrite(1);
        LedErrorWrite(1);
        HC595_ShiftOut(NULL,2,1);
        vTaskDelay(delay/portTICK_PERIOD_MS);
        LedStatusWrite(0);
        LedErrorWrite(0);
        HC595_ShiftOut(NULL,2,1);
        vTaskDelay(delay/portTICK_PERIOD_MS);
    }
}

void TestReadSingleButton(EventGroupHandle_t evg, EventBits_t e, gpio_num_t gpio,char* logMessage)
{
    if(!gpio_get_level(gpio)){
        vTaskDelay(100/portTICK_PERIOD_MS);
        LCDI2C_Print(logMessage,0,2);
        ESP_LOGI("LED Btn","%s",logMessage);
        while (!gpio_get_level(gpio))
        {
            vTaskDelay(50/portTICK_PERIOD_MS);
        }
        xEventGroupSetBits(evg,e);
    }
}