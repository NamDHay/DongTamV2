#ifndef _GUI_NAVIGATION_H_
#define _GUI_NAVIGATION_H_
#include "../ShareVar.h"
#include "GUI/GUI.h"
#include "BoardParameter.h"

/**
 * @brief POINTER_SLOT Ô dùng để hiển thị con trỏ trên màn hình, sẽ chiếm dụng một cột 
 * trước tên thông số và 1 cột trước giá trị của các thông số trên màn hình
 * 
 */
#define POINTER_SLOT 1 

/**
 * @brief Cho biết hiện tại con trỏ đang trỏ tới tên thông số hay giá trị của nó
 * 
 */
typedef enum {
    IS_KEYWORD = 1,
    IS_VALUE,
}PointerNow; 

/**
 * @brief Trang hiển thị, hiện tại không dùng
 * 
 */
typedef enum {
    PAGE_START,
    PAGE_SETTING,
    PAGE_CONTROL,
    PAGE_RUN,
    PAGE_END,
}Page; 

/**
 * @brief Điều hướng hiển thị màn hình LCD
 * @param pX Vị trí cột hiển thị con trỏ trên màn hình
 * @param pY Vị trí hàng hiển thị con trỏ 
 * @param page chuyển trang tiếp theo
 * @param pNow Cho biết con trỏ đang trỏ tới tên thông số(key) hay giá trị của thông số đó(value)
 * @param param thông số hiện tại đang trỏ tới để thao tác, lấy từ BoardParameter.h
 */
typedef struct GUI_NAV{
    uint8_t pX;
    uint8_t pY;
    Page page;
    PointerNow pNow;
    ParamIndex param;
    uint8_t orderDisplayIndex;
}GUI_NAV;

uint8_t GUINAV_GetPage();
/**
 * @brief Trả về thứ tự (index) của biến orderToDisplay trong GUI.c mà con trỏ đang trỏ tới, 
 * không phải thứ tự (index) các thông số trong BoardParameter
 * Ví dụ: con trỏ GUI đang trỏ tới phần tử (index) thứ 0 trong mảng orderToDisplay của GUI.c 
 * thì có giá trị là INDEX_PARAM_CODE thuộc enum ParamIndex trong BoardPameter.h, đây là giá trị mapping 
 * cho giống với bảng điều khiển của Đồng Tâm.
 * Để lấy được thông số thực sự mà con trỏ đang trỏ tới thì phải viết như sau:
 * paramOrderToDisplay[GUINAV_GetOrderToDisplayIndex]
 */
ParamIndex GUINAV_GetOrderDisplayIndex();
// Vị trí con trỏ trên màn hình LCD theo cột
uint8_t GUINAV_GetPointerPosX();
// Vị trí con trỏ trên màn hình LCD theo hàng 
uint8_t GUINAV_GetPointerPosY();

/**
 * @brief Cho biết hiện tại con trỏ đang trỏ tới tên thông số hay giá trị của nó
 * 
 * @return uint8_t 
 */
PointerNow GUINAV_GetCurrentSelected();
void GUINAV_GetEvent(EventBits_t e);
esp_err_t GUINAV_SetPointerPosX(uint8_t X);
esp_err_t GUINAV_SetPointerPosY(uint8_t Y);
void GUINAV_SetCurrentSelected(PointerNow pNow);
/**
 * @brief Lấy thông số được map trong bảng Đồng Tâm hay chính là paramOrderDisplay
 * Để lấy giá trị của paramOrderDisplay hay index thực sự của BoardParameter thì dùng GUINAV_GetParamDisplay(GUINAV_GetOrderDisplayIndex());
 * @param DisplayIndex Thứ tự hiển thị thông số của paramOrderDisplay
 * @return ParamIndex 
 */
ParamIndex GUINAV_GetParamDisplay(uint8_t DisplayIndex);


#endif