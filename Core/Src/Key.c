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

#define KEY(x) HAL_GPIO_ReadPin(GPIOB, x)

extern float Weight;
extern uint32_t SaveCount;
extern uint8_t WeightData[4];
uint8_t SaveResult = 0;  // 0: 无操作, 1: 保存成功, 2: 重量为负

Key_t Key_press = 
{
    .State = KEY_UP,
    .Last_time = 0
};

/************************************************************************************************************
 * 函数名称: Key_scan
 * 函数功能: 按键扫描
 * 输入参数: 无
 * 输出参数: Key_press.State
 * 返回值: 当前按键状态
************************************************************************************************************/
uint8_t Key_scan(uint16_t Input_key)
{
    uint8_t Input_Pin = KEY(Input_key);
    uint32_t Input_Time = HAL_GetTick();

    switch (Key_press.State)
    {
        //按键未按下
        case KEY_UP:
            if (Input_Pin == 1)
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
            if (Input_Pin == 1)
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
            if (Input_Pin == 0)
            {
                Key_press.State = KEY_DOWN;//检测到一直按下则一直为按下状态
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
            if (Input_Pin == 0)
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
            switch (Input_key) 
            {
                case Key1:
                    //按键 1 任务：保存当前数据到 W25Q64
                    if (Weight >= 0.0f)
                    {
                        WeightData[0] = ((uint8_t *)&Weight)[0];
                        WeightData[1] = ((uint8_t *)&Weight)[1];
                        WeightData[2] = ((uint8_t *)&Weight)[2];
                        WeightData[3] = ((uint8_t *)&Weight)[3];
                        
                        if (SaveCount % 1024 == 0)
                        {
                            W25Q64_SectorErase(SaveCount * 4);
                        }
                        
                        W25Q64_PageProgram(SaveCount * 4, WeightData, 4);
                        SaveCount++;
                        
                        SaveResult = 1;  // 保存成功
                    }
                    else
                    {
                        SaveResult = 2;  // 重量为负
                    }
                    break;
            }
            Key_press.State = KEY_UP;
            break;
    
    default:
        break;
    }

    return Key_press.State;
}

