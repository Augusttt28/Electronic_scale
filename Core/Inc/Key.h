#ifndef _KEY_H
#define _KEY_H

#include "main.h"
#include "gpio.h"

#define Key1 Key1_Pin
#define Key_Delay_time 20

//枚举，显示按键所有的状态
typedef enum
{
    KEY_UP = 0,
    KEY_DOWN_Dely,
    KEY_DOWN,
    KEY_UP_Delay,
    KEY_PROCESS_TASK
}Key_state;

//状态机状态结构体
typedef struct 
{
    Key_state State;//当前状态
    uint32_t Last_time;//每一步处理后的时间
}Key_t;

uint8_t Key_scan(uint16_t Input_key);


#endif 
