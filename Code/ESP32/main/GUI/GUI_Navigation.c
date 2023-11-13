#include "GUI_Navigation.h"
#include "GUI.h"

/**
 * @brief Thứ tự hiển thị các thông số trên màn hình khớp với giao diện gốc của Đồng Tâm
 * Đây không phải thứ tự thực sự của thông số trong BoardParameter.h mà là thứ tự đã được mapping lại
 * 
 */
ParamIndex paramOrderToDisplay[] = {
    INDEX_START_PARAM,//nothing to do with this
    INDEX_LANGUAGE,
    INDEX_PARAM_CODE,
    INDEX_DISPLAY_RANGE,
    INDEX_TRIG_VAN,
    INDEX_DP_LOW,
    INDEX_DP_HIGH,
    INDEX_DP_WARN,
    INDEX_PULSE_TIME,
    INDEX_INTERVAL_TIME,
    INDEX_CYCLE_INTERVAL_TIME,
    INDEX_TOTAL_VAN,
    INDEX_DOWN_TIME_CYCLE,
    INDEX_ODC_HIGH,
    INDEX_ODC_LOW,
    INDEX_OPERATE_HOURS,
    INDEX_SERV_RUN_HOURS,
    INDEX_SERV_RUN_HOURS_ALARM,
    INDEX_ODC_CLEAN_MODE,
    INDEX_TECH_CODE,
    INDEX_DP_MODE,
    INDEX_TEST_MODE,
    INDEX_DISPLAY_CONTRAST,
    INDEX_END_PARAM,
};

/**
 * @brief Khởi tạo giá trị ban đầu cho đối tượng điều hướng trên màn hình
 * 
 */
GUI_NAV guiNav = {
    .pX = 0,
    .pY = 0,
    .page = PAGE_SETTING,
    .param = INDEX_LANGUAGE,
    .pNow = IS_KEYWORD,
    .orderDisplayIndex = 1,
};

// Hiện tại chưa sử dụng tính năng này
void NextPage(){
    // guiNav.page++;
    // if(guiNav.page == PAGE_END) guiNav.page = PAGE_START + 1;
}

/**
 * @brief Trỏ tới thông số tiếp theo trong bảng, cuộn xuống thông số khi con trỏ 
 * nằm ở hàng dưới cùng
 * 
 */
void PointToNextParam(){
    xEventGroupClearBits(evgGUI,EVT_PARAM_SCROLL_UP);
    ParamIndex orderDisplayIndex = GUINAV_GetOrderDisplayIndex(); 
    orderDisplayIndex ++;
    ParamIndex paramNO = paramOrderToDisplay[orderDisplayIndex];
    int8_t pY = (int8_t) GUINAV_GetPointerPosY();
    if(paramNO == INDEX_END_PARAM) return;
    guiNav.orderDisplayIndex = orderDisplayIndex;
    guiNav.param = paramNO;  
    // if param reach the end of the list, keep the same value
    // not roll back pointer, keep it at the latest row
    if(pY == LCD_ROWS - 1) {
        xEventGroupSetBits(evgGUI,EVT_PARAM_SCROLL_DOWN);
        return;
    } else pY++;
    guiNav.pY = (uint8_t)pY;
}

/**
 * @brief Trỏ tới thông số trước đó trong bảng, cuộn lên thông số khi con trỏ 
 * nằm ở hàng trên cùng
 * 
 */
void PointToPrevParam(){
    xEventGroupClearBits(evgGUI,EVT_PARAM_SCROLL_DOWN);
    ParamIndex orderDisplayIndex = GUINAV_GetOrderDisplayIndex(); 
    orderDisplayIndex --;
    ParamIndex paramNO = paramOrderToDisplay[orderDisplayIndex];
    int pY = (int) GUINAV_GetPointerPosY();
    if(paramNO == INDEX_START_PARAM) return;
    guiNav.orderDisplayIndex = orderDisplayIndex;
    guiNav.param = paramNO;
    // if param reach the start of the list, keep the same value
    if(!pY) {
        xEventGroupSetBits(evgGUI,EVT_PARAM_SCROLL_UP);
        return;
    } else pY--;
    guiNav.pY = (uint8_t)pY;
}

void SetPointerNowIsKeyword(){
    guiNav.pNow = IS_KEYWORD;
    guiNav.pX = 0;
}
void SetPointerNowIsValue(){
    guiNav.pNow = IS_VALUE;
    guiNav.pX = LENGTH_OF_PARAM + 1;
}
void IncreaseValue(){
    EventBits_t e = xEventGroupGetBits(evgGUI);
    // if not happen event above threshold, bit EVT_INCREASE_VALUE will be set
    if(!CHECKFLAG(e,EVT_VALUE_ABOVE_THRESHOLD)) xEventGroupSetBits(evgGUI,EVT_INCREASE_VALUE);
}
void DecreaseValue(){
    EventBits_t e = xEventGroupGetBits(evgGUI);
    // if not happen event below threshold, bit EVT_DECREASE_VALUE will be set
    if(!CHECKFLAG(e,EVT_VALUE_BELOW_THRESHOLD)) xEventGroupSetBits(evgGUI,EVT_DECREASE_VALUE);
}

void SaveValue()
{
    ESP_LOGI("NAVSaveVal","Pass");
    EventBits_t e = xEventGroupGetBits(evgGUI);
    if(!CHECKFLAG(e,EVT_SET_VALUE_TO_FLASH)) xEventGroupSetBits(evgGUI,EVT_SET_VALUE_TO_FLASH); 
}

/**
 * @brief Xử lý sự kiện từ nút nhấn nhận được ở TaskScanButton
 * 
 * @param eventToHandle tên của sự kiện nút nhấn nhận được cần xử lý
 * @param eventName danh sách các sự kiện nút nhấn có thể được xử lý bởi hàm này quy định trong GUI.h
 * @param fKEY hàm xử lý sự kiện nếu con trỏ đang trỏ tới tên thông số(key)
 * @param fVALUE hàm xử lý sự kiện nếu con trỏ đang trỏ tới giá trị thông số(value)
 */
void HandleEvent(EventBits_t eventToHandle, EventBits_t eventName,void(*fKEY)(),void(*fVALUE)()){
    if(eventToHandle == eventName){
        if(guiNav.pNow == IS_KEYWORD) {
            fKEY();
        }
        else if(guiNav.pNow == IS_VALUE) {
            fVALUE();
        }
    }
}
/**
 * @brief Trả về sự kiện khi nhấn nút EventBits_t e
 * Sự kiện nhấn nút xảy ra ở TaskScanButton thuộc file LedButton.c, dùng TaskNotify thông báo cho GUITask và được chứa trong EventBits_t e
 * GUITask sẽ chờ sự kiện nhấn nút trong hàm xTaskNotifyWait, khi nhấn thì GUITask sẽ gọi tới hàm này để kiểm tra sự kiện và truyền 
 * tham số đầu vào là EventBits_t e nhận được từ TaskScanButton
 * 
 */
void GUINAV_GetEvent(EventBits_t e)
{
    HandleEvent(e,EVT_BTN_MENU,&NextPage,&SetPointerNowIsKeyword);
    HandleEvent(e,EVT_BTN_SET,&SetPointerNowIsValue,&SaveValue);
    HandleEvent(e,EVT_BTN_UP,&PointToPrevParam,&IncreaseValue);
    HandleEvent(e,EVT_BTN_DOWN_RIGHT,&PointToNextParam,&DecreaseValue);
    HandleEvent(e,EVT_LCD_RESET,&GUI_resetLCD,&GUI_resetLCD);
}


PointerNow GUINAV_GetCurrentSelected(){return guiNav.pNow;} 
uint8_t GUINAV_GetPage(){return guiNav.page;}
ParamIndex GUINAV_GetOrderDisplayIndex(){return guiNav.orderDisplayIndex;}
ParamIndex GUINAV_GetParamDisplay(uint8_t DisplayIndex){
    if(DisplayIndex >= INDEX_END_PARAM) return DisplayIndex;
    return paramOrderToDisplay[DisplayIndex];} 
uint8_t GUINAV_GetPointerPosX(){return guiNav.pX;}
uint8_t GUINAV_GetPointerPosY(){return guiNav.pY;} 

esp_err_t GUINAV_SetPointerPosX(uint8_t X)
{
    if(X > LCD_COLS - 1) return ESP_ERR_INVALID_ARG;
    guiNav.pX = X;
    return ESP_OK;
}

esp_err_t GUINAV_SetPointerPosY(uint8_t Y)
{
    if(Y > LCD_ROWS - 1) return ESP_ERR_INVALID_ARG;
    guiNav.pY = Y;
    return ESP_OK;
}
void GUINAV_SetCurrentSelected(PointerNow pNow)
{
    guiNav.pNow = pNow;
}