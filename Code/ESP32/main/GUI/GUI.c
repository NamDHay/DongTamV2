
#include "GUI.h"

#include "../BoardParameter.h"
#include "GUI_Navigation.h"
#include "LedButton.h"
#include "../MessageHandle/MessageHandle.h"
EventGroupHandle_t evgGUI;
LCDI2C lcdGUI;
TaskHandle_t taskGUIHandle;
GUIParam_t guiParam[LCD_ROWS];

/**
 * @brief Dùng để nhận thông tin từ các task khác thông qua phương pháp TaskNotify
 * Đây là đối số truyền vào khi khởi tạo GUITask dùng hàm xTaskCreate
 * 
 * @return TaskHandle_t* 
 */
TaskHandle_t* GUI_GetTaskHandle(){ return &taskGUIHandle;}

/**
 * @brief Tên thông số sẽ hiển thị trên màn hình LCD
 * 
 */
const char *paramText[]={
    "StartParam", //not use
    "Total Van :",
    "Down T Cyc:",
    "Clean Mode:",
    "TestMode  :",
    "Contrast  :",
    "DP-Low    :",
    "DP-High   :",
    "DP-Alarm  :",
    "ODC High  :",
    "ODC Low   :",
    "Pulse Time:",
    "Inter Time:",
    "Cycle Time:",
    "OperHours :",
    "Serv Run H:",
    "SerH Alarm:",
    // special param to handle
    "StringOffset",//not use
    "Language  :",
    "dpDisRange:",
    "Param code:",
    "Tech code :",
    "DP Mode   :",
    "Trig Van  :",
    " ",
};




void PrintNavigation();
uint8_t CountLengthValue(uint32_t value);
uint8_t CheckValueIsLimit(uint32_t *value, uint32_t valLowLimit, uint32_t valHighLimit, EventGroupHandle_t *eventLimit);
void GUI_ScrollUpDown(uint8_t paramNO);
void GUI_Manage();
void GUI_SaveValueToFlash();
void GUI_LCD_DeletePreviousValue(char *strToPrint,uint8_t lengthOfValue);
void GUI_ShowValueInt();
void GUI_ShowValueString();

/**
 * @brief Mỗi khi nhận được thông báo từ task khác gửi qua (ví dụ như task đọc nút nhấn và gửi sự kiện nhấn nút)
 * thì task này sẽ chạy các hàm bên dưới để điều khiển giao diện màn hình
 * GUINAV_GetEvent: đảm nhận việc xử lý sự kiện nút nhấn
 * 
 * @param pvParameter 
 */
void GUITask(void *pvParameter)
{
    EventBits_t e;
    while(1){
        if(xTaskNotifyWait(pdFALSE,pdTRUE,&e,10/portTICK_PERIOD_MS)){
            ESP_LOGI("GUITask","Get notify");
            GUI_ClearPointer();
            GUINAV_GetEvent(e);
            GUI_ShowPointer();
            PrintNavigation();
            GUI_Manage();
            vTaskDelay(100/portTICK_PERIOD_MS);
        }
    }
}

void GUI_Manage()
{
    PointerNow pNow = GUINAV_GetCurrentSelected();
    //Get the true value of index in BoardParameter, not the table show
    ParamIndex paramNO = GUINAV_GetParamDisplay(GUINAV_GetOrderDisplayIndex());
    ESP_LOGE("paramNO","%d",paramNO);
    if(pNow == IS_KEYWORD){
        GUI_ScrollUpDown(paramNO);
        GUI_ShowPointer();
    } else if (pNow == IS_VALUE){
        GUI_ShowValueInt();
        GUI_ShowValueString();
        GUI_SaveValueToFlash();
    }
}


/**
 * @brief Single function section
 *  
 */


void GUI_SendCommandToSTM32()
{
    ParamIndex paramNO = GUINAV_GetParamDisplay(GUINAV_GetOrderDisplayIndex());
    char s[30] = {0};
    switch (paramNO)
    {
        case INDEX_TRIG_VAN:
        MessageTxHandle(TX_TRIG_VAN,s);    
        break;
        case INDEX_CYCLE_INTERVAL_TIME:
        MessageTxHandle(TX_CYC_INTV_TIME,s);
        break;
        case INDEX_INTERVAL_TIME:
        MessageTxHandle(TX_INTERVAL_TIME,s);
        break;
        case INDEX_TOTAL_VAN:
        MessageTxHandle(TX_TOTAL_VAN,s);
        break;
        case INDEX_PULSE_TIME:
        MessageTxHandle(TX_PULSE_TIME,s);
        break;
        default:
        break;
    }
    ESP_LOGI("GUI_STM32","paramNO:%d,%s",paramNO,s);
    SendStringToUART(qSTM32Tx,s);
}

void GUI_SaveValueToFlash()
{
    esp_err_t err = ESP_OK;
    EventBits_t e = xEventGroupGetBits(evgGUI);
    if(CHECKFLAG(e,EVT_SET_VALUE_TO_FLASH)){
        xEventGroupClearBits(evgGUI,EVT_SET_VALUE_TO_FLASH);
        err = Brd_WriteParamToFlash();
        LCDI2C_Clear();
        vTaskDelay(20/portTICK_PERIOD_MS);
        LCDI2C_Print("Save data",0,0);
        if(err == ESP_OK) LCDI2C_Print("OK",0,1);
        else LCDI2C_Print("FAILED",0,1);
        GUI_SendCommandToSTM32();
        vTaskDelay(1500/portTICK_PERIOD_MS);
        GUI_LoadPage();

    }

}

/**
 * @brief Dùng để xử lý, hiển thị giá trị thông số kiểu string lên màn hình
 * 
 */
void GUI_ShowValueString()
{
    ParamIndex paramNO = GUINAV_GetParamDisplay(GUINAV_GetOrderDisplayIndex());
    if(paramNO < INDEX_STRING_PARAM_OFFSET) return;
    uint8_t pY = GUINAV_GetPointerPosY();
    uint8_t pX = GUINAV_GetPointerPosX();
    uint32_t stepChange = Brd_GetParamStepChange(paramNO);
    uint8_t valueStringIndex = Brd_GetParamStringValueIndex(paramNO);
    char s[10] = {0};

    // Remove old valueString
    strncpy(s,"    ",strlen(Brd_ConvertStringValueIndexToString(valueStringIndex)));
    ESP_ERROR_CHECK(LCDI2C_Print(s,pX+POINTER_SLOT,pY));
    // Waiting for event increase and decrease value 
    EventBits_t BitToWait = EVT_INCREASE_VALUE|EVT_DECREASE_VALUE ;
    EventBits_t e = xEventGroupWaitBits(evgGUI,BitToWait, pdTRUE,pdFALSE,0);
    if(CHECKFLAG(e,EVT_INCREASE_VALUE)) valueStringIndex +=stepChange;
    if(CHECKFLAG(e,EVT_DECREASE_VALUE)) valueStringIndex -=stepChange;
    Brd_SetParamStringValueIndex(paramNO,&valueStringIndex,NULL);
    ESP_LOGI("ShowValueString","currentVal:%u,BrdVal:%u",valueStringIndex,Brd_GetParamStringValueIndex(paramNO));
    strcpy(s,Brd_ConvertStringValueIndexToString(valueStringIndex));
    ESP_ERROR_CHECK(LCDI2C_Print(s,pX+POINTER_SLOT,pY));
    
}

/**
 * @brief Dùng để xử lý giá trị thông số có kiểu dữ liệu integer trên màn hình
 * 
 */
void GUI_ShowValueInt()
{
    char unit[5] = {0};
    static uint8_t preValLen = 0;
    char s[8]={0};
    // check if unit is NULL or not, if not then copy it to unit array variable
    ParamIndex paramNO = GUINAV_GetParamDisplay(GUINAV_GetOrderDisplayIndex());
    if(paramNO >= INDEX_STRING_PARAM_OFFSET) return;
    uint8_t pY = GUINAV_GetPointerPosY();
    uint8_t pX = GUINAV_GetPointerPosX();
    uint32_t stepChange = Brd_GetParamStepChange(paramNO);
    uint16_t valLowLimit = Brd_GetMinLimit(paramNO);
    uint16_t valHighLimit = Brd_GetMaxLimit(paramNO);
    uint32_t value = Brd_GetParamIntValue(paramNO);

    // Delete only number if previous value length equal current value, else delete number and unit 
    if(preValLen != CountLengthValue(value)) {
        if(Brd_GetUnit(paramNO)) strcpy(unit,Brd_GetUnit(paramNO));
        preValLen = CountLengthValue(value);
        strncpy(s,"       ",(LCD_COLS - (pX + POINTER_SLOT)));
        ESP_ERROR_CHECK(LCDI2C_Print(s,pX+POINTER_SLOT,pY));
    } else {
        // Clear previous value
        strncpy(s,"       ",preValLen);
        ESP_ERROR_CHECK(LCDI2C_Print(s,pX+POINTER_SLOT,pY));
    }

    // Waiting for event increase and decrease value 
    EventBits_t BitToWait = EVT_INCREASE_VALUE|EVT_DECREASE_VALUE ;
    EventBits_t e = xEventGroupWaitBits(evgGUI,BitToWait, pdTRUE,pdFALSE,0);
    if(CHECKFLAG(e,EVT_INCREASE_VALUE)) value +=stepChange;
    if(CHECKFLAG(e,EVT_DECREASE_VALUE)) value -=stepChange;

    /* Need to repeat this step again because value length can be change after increasing or decreasing value
    delete only number if previous value length equal current value, else delete number and unit 
    */ 
    if(preValLen != CountLengthValue(value)) {
        if(Brd_GetUnit(paramNO)) strcpy(unit,Brd_GetUnit(paramNO));
        preValLen = CountLengthValue(value);
        strncpy(s,"       ",(LCD_COLS - (pX + POINTER_SLOT))); // delete all remain slot
        ESP_ERROR_CHECK(LCDI2C_Print(s,pX+POINTER_SLOT,pY));
    } else {
        // Clear previous value
        strncpy(s,"       ",preValLen);
        ESP_ERROR_CHECK(LCDI2C_Print(s,pX+POINTER_SLOT,pY));
    }

    // print unit and number if unit is copied, else print only number
    if(unit[0]) sprintf(s,"%lu%s",value,unit);
    else sprintf(s,"%lu",value);
    ESP_ERROR_CHECK(LCDI2C_Print(s,pX+POINTER_SLOT,pY));
    CheckValueIsLimit(&value,valLowLimit,valHighLimit,&evgGUI);
    Brd_SetParamInt(paramNO,value,NULL);
}

void GUI_ScrollUpDown(uint8_t paramNO)
{
    EventBits_t e = xEventGroupGetBits(evgGUI);
    if((CHECKFLAG(e,EVT_PARAM_SCROLL_UP))){
        GUI_LoadPage();
    }
    // if current pointer pointed to the last row and param is not the last param
    else if((CHECKFLAG(e,EVT_PARAM_SCROLL_DOWN))){
        GUI_LoadPage();
    }
}

/**
 * @brief Tính toán số ô cần xóa giá trị value trước đó, xóa tới 6 hàng
 * 
 * @param value Giá trị cần xóa trước khi update giá trị mới vào 
 * @return uint8_t Số ô màn hình LCD cần xóa mà giá trị trước đó chiếm dụng
 */
uint8_t CountLengthValue(uint32_t value){
    uint8_t lenPrevVal = 0;
    if(value < 10) lenPrevVal = 1;
    else if(value < 100) lenPrevVal = 2; // 2 digit to clear
    else if(value < 1000) lenPrevVal = 3; // 3 digit to clear
    else if(value < 10000) lenPrevVal = 4; // 4 digit to clear
    else if(value < 100000) lenPrevVal = 5; // 5 digit to clear
    else if(value < 1000000) lenPrevVal = 6; // 6 digit to clear
    return lenPrevVal;
}

/**
 * @brief Kiểm tra giá trị đầu vào có vượt ngưỡng thấp và cao hay không
 * Nếu có thì gán ngưỡng vào giá trị đầu vào 
 * @param value Giá trị cần kiểm tra ngưỡng
 * @param valLowLimit Ngưỡng thấp của thông số
 * @param valHighLimit Ngưỡng cao của thông số
 * @param eventLimit event group để bật cờ sự kiện giá trị vượt ngưỡng
 * @return uint8_t 0 là không vượt ngưỡng, 1 ngưỡng cao và 2 là ngưỡng thấp
 */
uint8_t CheckValueIsLimit(uint32_t *value, uint32_t valLowLimit, uint32_t valHighLimit, EventGroupHandle_t *eventLimit)
{
    uint32_t temp = *value;
    // if value is above limit, set it to limit and set eventbit in evgGUI
    if(temp >= valHighLimit){
        *value = valHighLimit;
        xEventGroupSetBits(*eventLimit,EVT_VALUE_ABOVE_THRESHOLD);
        //avoid deadlock
        xEventGroupClearBits(*eventLimit,EVT_VALUE_BELOW_THRESHOLD);
        ESP_LOGE("GUI_Manage","high limit");
        return 1;
    } 
    else if(temp <= valLowLimit){
        *value = valLowLimit;
        xEventGroupSetBits(*eventLimit,EVT_VALUE_BELOW_THRESHOLD);
        //avoid deadlock
        xEventGroupClearBits(*eventLimit,EVT_VALUE_ABOVE_THRESHOLD);
        ESP_LOGE("GUI_Manage","low limit");
        return 2;
    }
    else {
        *value = temp;   
        EventBits_t e = xEventGroupGetBits(*eventLimit);
        //if none of conditions above is passed, it mean value is not saturated anymore, so check and clear bit control threshold
        if(CHECKFLAG(e,EVT_VALUE_ABOVE_THRESHOLD)) xEventGroupClearBits(*eventLimit,EVT_VALUE_ABOVE_THRESHOLD);
        else if(CHECKFLAG(e,EVT_VALUE_BELOW_THRESHOLD)) xEventGroupClearBits(*eventLimit,EVT_VALUE_BELOW_THRESHOLD);
        // ESP_LOGI("GUI_Manage","not limit");
    }
    return 0;
}

/**
 * @brief Load giá trị của thông số vào trong GUIParam 
 * 
 * @param gp 
 * @param paramNO 
 */
void GUI_GetParam(GUIParam_t *gp, ParamIndex paramNO)
{
    //Get value integer
    if(paramNO >= INDEX_TOTAL_VAN && paramNO <= INDEX_SERV_RUN_HOURS_ALARM){
        gp->index = paramNO;
    }
}

void GUI_PrintParam(uint8_t index, uint8_t row)
{
    if(index >= INDEX_END_PARAM) return;
    ESP_LOGI("PrintParam","Index is:%u,row:%u",index,row);
    char unit[5] = {0};
    // check if unit is NULL or not, if not then copy it to unit array variable
    if(Brd_GetUnit(index)) {
        strcpy(unit,Brd_GetUnit(index)); 
        ESP_LOGI("PrintParam","Unit: %s",Brd_GetUnit(index));
    }
    LCDI2C_Print(paramText[index],POINTER_SLOT,row);
    
    char StringValue[20];
    // if no string is copied to unit, strlen would be zero
    if(!strlen(unit)) {
        // handle string value
        if(index > INDEX_STRING_PARAM_OFFSET && index < INDEX_END_PARAM){
            if(Brd_GetParamStringValueIndex(index)) sprintf(StringValue,"%s",Brd_ConvertStringValueIndexToString(Brd_GetParamStringValueIndex(index)));
            else ESP_LOGE("PrintParam","No string found ");
        }
        // handle int value non unit
        else {
            sprintf(StringValue,"%lu",Brd_GetParamIntValue(index));
        }
    }
    else sprintf(StringValue,"%lu%s",Brd_GetParamIntValue(index),unit);
    // pointer slot for pointer of text and slot for point of value
    LCDI2C_Print(StringValue,POINTER_SLOT + LENGTH_OF_PARAM + POINTER_SLOT,row);
}

void GUI_LoadPage()
{
    ParamIndex paramNO;
    LCDI2C_Clear();
    vTaskDelay(20/portTICK_PERIOD_MS);
    for(uint8_t i=0;i<LCD_ROWS;i++){
        paramNO = GUINAV_GetParamDisplay(GUINAV_GetOrderDisplayIndex()+i);
        GUI_PrintParam(paramNO,i);
    }
    GUINAV_SetCurrentSelected(IS_KEYWORD);
    GUINAV_SetPointerPosX(0);
    GUINAV_SetPointerPosY(0);
    xEventGroupClearBits(evgGUI,EVT_PARAM_SCROLL_DOWN);
    GUI_ShowPointer();
}

/**
 * @brief Hiển thị các thông số lên màn hình LCD theo thứ tự trong bảng, không tuân theo thứ tự của ParamIndex
 * 
 */
void GUI_LoadPageAtInit()
{
    ParamIndex paramNO;
    // each parameter will be placed in each rows of LCD, avoid i=0 
    for(uint8_t i=0;i<LCD_ROWS;i++){
        paramNO = GUINAV_GetParamDisplay(GUINAV_GetOrderDisplayIndex()+i); 
        GUI_PrintParam(paramNO,i);
    }
}

void GUI_ShowPointer(){LCDI2C_Print(">",GUINAV_GetPointerPosX(),GUINAV_GetPointerPosY());}
void GUI_ClearPointer(){LCDI2C_Print(" ",GUINAV_GetPointerPosX(),GUINAV_GetPointerPosY());}

/*
    End Single function section
*/


/**
 * @brief Debug section
 * 
 */


void PrintNavigation()
{
    ESP_LOGI("GUI_NAV","orderDisplayIndex: %u",GUINAV_GetOrderDisplayIndex());
    ESP_LOGI("GUI_NAV","param: %u",GUINAV_GetParamDisplay(GUINAV_GetOrderDisplayIndex()));
}
/*
    End Debug section
*/


/**
 * @brief Init section
 * 
 */



void PressureIndicator_Init()
{
    PI_Init();
    PI_SetLevel(0);
}

esp_err_t LCD_init()
{
    esp_err_t err = ESP_OK;
    err = LCDI2C_Config(&lcdGUI);
    LCDI2C_TurnOnBackLight();
    return err;
}

void GUI_resetLCD()
{
    LCD_init();
    GUI_LoadPage();
}

esp_err_t ButtonInit()

{
    gpio_config_t cfg = {
    .mode = GPIO_MODE_INPUT,
    .pin_bit_mask = BT1_MASK |
                    BT2_MASK |
                    BT3_MASK|
                    BT4_MASK,
    };
    return gpio_config(&cfg);
}

void GuiInit(){
    evgGUI = xEventGroupCreate();
    ButtonInit();
    LCD_init();
    PressureIndicator_Init();
    // GUI_ShowPointer();
    // GUI_LoadPageAtInit();
}

/*
    End Init section
*/
