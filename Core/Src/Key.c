/************************************************************************************************************
 * Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * Date: 2025-11-03 15:49:23
 * LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * LastEditTime: 2025-11-04 00:06:39
 * FilePath: \MDK-ARMf:\stm32project\Stm32project\Freertos-text\LED\Hardware\Key.c
 * Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
************************************************************************************************************/

#include "Key.h"
#include "main.h"
#include "gpio.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdint.h>
#include "W25Q64.h"
#include "OLED.h"
#include "hx711.h"

#define KEY(x) HAL_GPIO_ReadPin(GPIOB, x)

extern float Weight;
extern uint32_t SaveCount;
extern uint8_t WeightData[4];
extern float UnitPrice;
extern float TotalPrice;
uint8_t SaveResult = 0;  // 0: 无操作, 1: 保存成功, 2: 重量为负

Key_t Key_press = 
{
    .State = KEY_UP,
    .Last_time = 0
};


Key_t Key2_press = 
{
    .State = KEY_UP,
    .Last_time = 0
};


Key_t Key3_press = 
{
    .State = KEY_UP,
    .Last_time = 0
};

Key_t Key4_press = 
{
    .State = KEY_UP,
    .Last_time = 0
};


/************************************************************************************************************
 * 函数名称: Key1_scan
 * 函数功能: 按键扫描
 * 输入参数: 无
 * 输出参数: Key_press.State
 * 返回值: 当前按键状态
************************************************************************************************************/
uint8_t Key1_scan(uint16_t Input_key)
{
    uint8_t Input_Pin = KEY(Input_key);
    uint32_t Input_Time = HAL_GetTick();

    switch (Key_press.State)
    {
        //按键未按下
        case KEY_UP:
            if (Input_Pin == 0)
            {
                Key_press.State = KEY_UP;//检测到一直按下，一直为按键未按下状态
            }
            else
            {
                //检测到按下，进入按下消抖状态
                Key_press.State = KEY_DOWN_Dely;
                Key_press.Last_time = Input_Time;
            }
            break;    
        
        //监测到按键按下，进入按下消抖
        case KEY_DOWN_Dely:
            if (Input_Pin == 0)
            {
                Key_press.State = KEY_UP;//误触发
            }
            else
            {
                if (Input_Time > Key_press.Last_time + Key_Delay_time)//到达消抖时间
                {
                    Key_press.State = KEY_DOWN;
                    //还能干其他事情。。。
                }               
            }
            break;
        
        //按键按下状态
        case KEY_DOWN:
            if (Input_Pin == 1)
            {
                Key_press.State = KEY_DOWN;//检测到一直按下则一直为按下状态
                //按键 1 任务：保存当前数据到 W25Q64
                if (Weight >= 0.0f)
                {
                    WeightData[0] = ((uint8_t *)&Weight)[0];
                    WeightData[1] = ((uint8_t *)&Weight)[1];
                    WeightData[2] = ((uint8_t *)&Weight)[2];
                    WeightData[3] = ((uint8_t *)&Weight)[3];
                    
                    WeightData[4] = ((uint8_t *)&UnitPrice)[0];
                    WeightData[5] = ((uint8_t *)&UnitPrice)[1];
                    WeightData[6] = ((uint8_t *)&UnitPrice)[2];
                    WeightData[7] = ((uint8_t *)&UnitPrice)[3];
                    
                    WeightData[8] = ((uint8_t *)&TotalPrice)[0];
                    WeightData[9] = ((uint8_t *)&TotalPrice)[1];
                    WeightData[10] = ((uint8_t *)&TotalPrice)[2];
                    WeightData[11] = ((uint8_t *)&TotalPrice)[3];
                    
                    WeightData[12] = ((uint8_t *)&history_index)[0];
                    WeightData[13] = ((uint8_t *)&history_index)[1];
                    WeightData[14] = ((uint8_t *)&history_index)[2];
                    WeightData[15] = ((uint8_t *)&history_index)[3];
                    
                    if (SaveCount % 1024 == 0)
                    {
                        W25Q64_SectorErase(SaveCount * 16);
                    }
                    
                    W25Q64_PageProgram(SaveCount * 16, WeightData, 16);
                    SaveCount++;
                    history_index++;
                    SaveResult = 1;
                }
                else
                {
                    SaveResult = 2;
                }                      
                if (Input_Time > Key_press.Last_time + Key_LongPress_time) 
                {
                    Key_press.State = KEY_LONG_PRESS;
                }
            }
            else
            {
                //检测到抬起则进入消抖
                Key_press.State = KEY_UP_Delay;
                Key_press.Last_time = Input_Time;
            }
            break;
        
        case  KEY_LONG_PRESS:
            if (Input_Pin == 1) 
            {
                Key_press.State = KEY_LONG_PRESS;//一直为长按状态
                //按键 1 任务：清空记录
                SaveCount = 0;
                W25Q64_ClearAllRecords();                                                   
            }
            else
            {
                //检测到抬起则进入消抖
                Key_press.State = KEY_UP_Delay;
                Key_press.Last_time = Input_Time;
            }
            break;
            
        //按键抬起消抖
        case KEY_UP_Delay:
            if (Input_Pin == 1)
            {
                Key_press.State = KEY_DOWN;//误触发
            }
            else
            {
                if (Input_Time > Key_press.Last_time + Key_Delay_time)//到达消抖时间
                {
                    Key_press.State = KEY_PROCESS_TASK;
                    //还能干其他事情
                }               
            }        
            break;
        
        //处理任务状态
        case KEY_PROCESS_TASK:
            
    
            Key_press.State = KEY_UP;
            break;
    
    default:
        break;
    }

    return Key_press.State;
}


/************************************************************************************************************
 * 函数名称: Key2_scan
 * 函数功能: 按键扫描
 * 输入参数: 无
 * 输出参数: Key_press.State
 * 返回值: 当前按键状态
************************************************************************************************************/
uint8_t Key2_scan(uint16_t Input_key)
{
    uint8_t Input_Pin = KEY(Input_key);
    uint32_t Input_Time = HAL_GetTick();

    switch (Key2_press.State)
    {
        //按键未按下
        case KEY_UP:
            if (Input_Pin == 0)
            {
                Key2_press.State = KEY_UP;//检测到一直按下，一直为按键未按下状态
            }
            else
            {
                //检测到按下，进入按下消抖状态
                Key2_press.State = KEY_DOWN_Dely;
                Key2_press.Last_time = Input_Time;
            }
            break;    
        
        //监测到按键按下，进入按下消抖
        case KEY_DOWN_Dely:
            if (Input_Pin == 0)
            {
                Key2_press.State = KEY_UP;//误触发
            }
            else
            {
                if (Input_Time > Key2_press.Last_time + Key_Delay_time)//到达消抖时间
                {
                    Key2_press.State = KEY_DOWN;
                    //还能干其他事情。。。
                }               
            }
            break;
        
        //按键按下状态
        case KEY_DOWN:
            if (Input_Pin == 1)
            {
                Key2_press.State = KEY_DOWN;//检测到一直按下则一直为按下状态
                if (current_display_state == DISPLAY_WEIGHING) 
                {
                    HX711_Tare();
                   
                }
                if (Input_Time > Key2_press.Last_time + Key_LongPress_time) 
                {
                    Key2_press.State = KEY_LONG_PRESS;
                }                                  
            }
            else
            {
                //检测到抬起则进入消抖
                Key2_press.State = KEY_UP_Delay;
                Key2_press.Last_time = Input_Time;
            }
            break;
        
        case  KEY_LONG_PRESS:
            if (Input_Pin == 1) 
            {
                Key2_press.State = KEY_LONG_PRESS;//一直为长按状态        
                 if (current_display_state == DISPLAY_WEIGHING) 
                {
                    OLED_Clear();
                    current_display_state = DISPLAY_HISTORY;
                }
                else if(current_display_state == DISPLAY_HISTORY) 
                {
                    OLED_Clear();
                    current_display_state = DISPLAY_WEIGHING;
                }                            
            }
            else
            {
                //检测到抬起则进入消抖
                Key2_press.State = KEY_UP_Delay;
                Key2_press.Last_time = Input_Time;
            }
            break;
            
        //按键抬起消抖
        case KEY_UP_Delay:
            if (Input_Pin == 1)
            {
                Key2_press.State = KEY_DOWN;//误触发
            }
            else
            {
                if (Input_Time > Key2_press.Last_time + Key_Delay_time)//到达消抖时间
                {
                    Key2_press.State = KEY_PROCESS_TASK;
                    //还能干其他事情
                }               
            }        
            break;
        
        //处理任务状态
        case KEY_PROCESS_TASK:              
            // HX711_Tare();           
            Key2_press.State = KEY_UP;
            break;
    
    default:
        break;
    }

    return Key2_press.State;
}

/************************************************************************************************************
 * 函数名称: Key3_scan
 * 函数功能: 按键扫描
 * 输入参数: 无
 * 输出参数: Key_press.State
 * 返回值: 当前按键状态
************************************************************************************************************/
uint8_t Key3_scan(uint16_t Input_key)
{
    uint8_t Input_Pin = KEY(Input_key);
    uint32_t Input_Time = HAL_GetTick();

    switch (Key3_press.State)
    {
        //按键未按下
        case KEY_UP:
            if (Input_Pin == 0)
            {
                Key3_press.State = KEY_UP;//检测到一直按下，一直为按键未按下状态
            }
            else
            {
                //检测到按下，进入按下消抖状态
                Key3_press.State = KEY_DOWN_Dely;
                Key3_press.Last_time = Input_Time;
            }
            break;    
        
        //监测到按键按下，进入按下消抖
        case KEY_DOWN_Dely:
            if (Input_Pin == 0)
            {
                Key3_press.State = KEY_UP;//误触发
            }
            else
            {
                if (Input_Time > Key3_press.Last_time + Key_Delay_time)//到达消抖时间
                {
                    Key3_press.State = KEY_DOWN;
                    //还能干其他事情。。。
                }               
            }
            break;
        
        //按键按下状态
        case KEY_DOWN:
            if (Input_Pin == 1)
            {
                Key3_press.State = KEY_DOWN;//检测到一直按下则一直为按下状态
                //按键3 任务：增加单价或增加查询索引
                if (current_display_state == DISPLAY_HISTORY) 
                {
                    save_index++;
                    if (save_index >= history_index) 
                    {
                        save_index = history_index;
                    }
                }
                else if (current_display_state == DISPLAY_WEIGHING) 
                {
                     UnitPrice += 1.0f;
                    if (UnitPrice > 999.0f)
                    {
                        UnitPrice = 999.0f;
                    }
                }                    
                if (Input_Time > Key3_press.Last_time + Key_LongPress_time) 
                {
                    Key3_press.State = KEY_LONG_PRESS;
                }
            }
            else
            {
                //检测到抬起则进入消抖
                Key3_press.State = KEY_UP_Delay;
                Key3_press.Last_time = Input_Time;
            }
            break;
        
        case  KEY_LONG_PRESS:
            if (Input_Pin == 1) 
            {
                Key3_press.State = KEY_LONG_PRESS;//一直为长按状态
                //按键3 任务：增加单价
                if (current_display_state == DISPLAY_WEIGHING) 
                {
                    UnitPrice += 3.0f;
                    if (UnitPrice > 999.0f)
                    {
                        UnitPrice = 999.0f;
                    }
                }               
            }
            else
            {
                //检测到抬起则进入消抖
                Key3_press.State = KEY_UP_Delay;
                Key3_press.Last_time = Input_Time;
            }
            break;
            
        //按键抬起消抖
        case KEY_UP_Delay:
            if (Input_Pin == 1)
            {
                Key3_press.State = KEY_DOWN;//误触发
            }
            else
            {
                if (Input_Time > Key3_press.Last_time + Key_Delay_time)//到达消抖时间
                {
                    Key3_press.State = KEY_PROCESS_TASK;
                    //还能干其他事情
                }               
            }        
            break;
        
        //处理任务状态
        case KEY_PROCESS_TASK:              
            //按键3 任务：增加单价
            
            Key3_press.State = KEY_UP;
            break;
    
    default:
        break;
    }

    return Key3_press.State;
}

/************************************************************************************************************
 * 函数名称: Key4_scan
 * 函数功能: 按键扫描
 * 输入参数: 无
 * 输出参数: Key_press.State
 * 返回值: 当前按键状态
************************************************************************************************************/
uint8_t Key4_scan(uint16_t Input_key)
{
    uint8_t Input_Pin = KEY(Input_key);
    uint32_t Input_Time = HAL_GetTick();

    switch (Key4_press.State)
    {
        //按键未按下
        case KEY_UP:
            if (Input_Pin == 0)
            {
                Key4_press.State = KEY_UP;//检测到一直按下，一直为按键未按下状态
            }
            else
            {
                //检测到按下，进入按下消抖状态
                Key4_press.State = KEY_DOWN_Dely;
                Key4_press.Last_time = Input_Time;
            }
            break;    
        
        //监测到按键按下，进入按下消抖
        case KEY_DOWN_Dely:
            if (Input_Pin == 0)
            {
                Key4_press.State = KEY_UP;//误触发
            }
            else
            {
                if (Input_Time > Key4_press.Last_time + Key_Delay_time)//到达消抖时间
                {
                    Key4_press.State = KEY_DOWN;
                    //还能干其他事情。。。
                }               
            }
            break;
        
        //按键按下状态
        case KEY_DOWN:
            if (Input_Pin == 1)
            {
                Key4_press.State = KEY_DOWN;//检测到一直按下则一直为按下状态
                //按键4 任务：减少单价或增加查询索引
                if (current_display_state == DISPLAY_HISTORY) 
                {
                    save_index--;
                    if (save_index <= 0) 
                    {
                        save_index = history_index;
                    }
                }
                else if (current_display_state == DISPLAY_WEIGHING) 
                {
                    UnitPrice -= 1.0f;
                    if (UnitPrice < 0.0f)
                    {
                        UnitPrice = 0.0f;
                    }                                   
                }
                
                if (Input_Time > Key4_press.Last_time + Key_LongPress_time) 
                {
                    Key4_press.State = KEY_LONG_PRESS;
                }
            }
            else
            {
                //检测到抬起则进入消抖
                Key4_press.State = KEY_UP_Delay;
                Key4_press.Last_time = Input_Time;
            }
            break;
        
        case  KEY_LONG_PRESS:
            if (Input_Pin == 1) 
            {
                Key4_press.State = KEY_LONG_PRESS;//一直为长按状态
                //按键4 任务：减少单价
                if (current_display_state == DISPLAY_WEIGHING) 
                {
                    UnitPrice -= 3.0f;
                    if (UnitPrice < 0.0f)
                    {
                        UnitPrice = 0.0f;
                    }
                }               
            }
            else
            {
                //检测到抬起则进入消抖
                Key4_press.State = KEY_UP_Delay;
                Key4_press.Last_time = Input_Time;
            }
            break;
            
        //按键抬起消抖
        case KEY_UP_Delay:
            if (Input_Pin == 1)
            {
                Key4_press.State = KEY_DOWN;//误触发
            }
            else
            {
                if (Input_Time > Key4_press.Last_time + Key_Delay_time)//到达消抖时间
                {
                    Key4_press.State = KEY_PROCESS_TASK;
                    //还能干其他事情
                }               
            }        
            break;
        
        //处理任务状态
        case KEY_PROCESS_TASK:
                                
            Key4_press.State = KEY_UP;
            break;
    
    default:
        break;
    }

    return Key4_press.State;
}

HistoryRecord* Key_GetHistoryRecord(uint32_t index)
{
    static HistoryRecord record;
    uint8_t data[16];
    uint32_t address = index * 16;
    
    W25Q64_ReadData(address, data, 16);
    
    record.weight = *((float *)&data[0]);
    record.unit_price = *((float *)&data[4]);
    record.total_price = *((float *)&data[8]);
    record.history_index = *((uint32_t *)&data[12]);
    
    return &record;
}
