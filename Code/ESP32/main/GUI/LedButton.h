#ifndef LED_BUTTON_H
#define LED_BUTTON_H

#define EVT_BTN_MENU (1<<0)
#define EVT_BTN_SET (1<<1)
#define EVT_BTN_UP (1<<2)
#define EVT_BTN_DOWN_RIGHT (1<<3)

#define LED_ERROR_MASK 10
#define LED_STATUS_MASK 11

#define BTN_MENU GPIO_NUM_36
#define BTN_SET GPIO_NUM_39
#define BTN_UP GPIO_NUM_34
#define BTN_DOWN_RIGHT GPIO_NUM_35

#define BT1_MASK (1ULL<<GPIO_NUM_36)
#define BT2_MASK (1ULL<<GPIO_NUM_39)
#define BT3_MASK (1ULL<<GPIO_NUM_34)
#define BT4_MASK (1ULL<<GPIO_NUM_35)
void LedErrorWrite(bool ledState);
void LedStatusWrite(bool ledState);
#endif