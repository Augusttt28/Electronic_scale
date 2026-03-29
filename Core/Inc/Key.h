#ifndef _KEY_H
#define _KEY_H

#include "main.h"
#include "gpio.h"

#define Key1 Key1_Pin
#define Key2 Key2_Pin
#define Key3 Key3_Pin
#define Key4 Key4_Pin
#define Key_Delay_time 20
#define Key_LongPress_time 500

//枚举，显示按键所有的状态
typedef enum
{
    KEY_UP = 0,
    KEY_DOWN_Dely,
    KEY_DOWN,
    KEY_LONG_PRESS,
    KEY_UP_Delay,
    KEY_PROCESS_TASK
}Key_state;

//显示状态枚举
typedef enum
{
    DISPLAY_WEIGHING = 0,
    DISPLAY_HISTORY = 1
}DisplayState;

//状态机状态结构体
typedef struct 
{
    Key_state State;//当前状态
    uint32_t Last_time;//每一步处理后的时间
}Key_t;

//历史记录结构体
typedef struct
{
    float weight;
    float unit_price;
    float total_price;
    uint32_t history_index;
}HistoryRecord;

extern DisplayState current_display_state;
extern uint8_t SaveResult;
extern float UnitPrice;
extern float TotalPrice;
extern uint32_t history_index;
extern uint32_t save_index;

uint8_t Key1_scan(uint16_t Input_key);
uint8_t Key2_scan(uint16_t Input_key);
uint8_t Key3_scan(uint16_t Input_key);
uint8_t Key4_scan(uint16_t Input_key);
HistoryRecord* Key_GetHistoryRecord(uint32_t index);


#endif 
