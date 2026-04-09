/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "hx711.h"
#include "Serial.h"
#include "W25Q64.h"
#include "Key.h"
#include <stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern const uint8_t BMP_ICON[];

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int32_t Rawval = 0;//HX711原始数据
float Weight = 0;//重量
uint32_t SaveCount = 0;//保存结果
uint8_t WeightData[16];//重量数据数组
uint32_t SaveDisplayTime = 0;//保存结果显示时间
float UnitPrice = 0.0f;//单价
float TotalPrice = 0.0f;//总价
DisplayState current_display_state = DISPLAY_WEIGHING;//显示模式
uint32_t history_index = 0;//历史记录索引
uint32_t save_index = 0;//查询记录索引
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  OLED_ShowString(2, 1, "Elector_SCALE");
  OLED_ShowString(3, 1, "Initializing...");
  HX711_Init();
  HX711_KalmanInit(0.05f, 0.1f, 0.0f);
  W25Q64_Init();
  // OLED_ShowString(1, 1, "Weight:");
  // 1. 先去皮（确保秤上无物品）
  while (!HX711_TareTimeout(5000))
  {
      OLED_Clear();
      OLED_ShowString(2, 1, "SCALE");
      OLED_ShowString(3, 1, "NOT CONNECT");
      HAL_Delay(500);
  }
  HAL_Delay(100);
  
  // // 2. 放置已知重量的物品进行校准（例如1000g）
  // HX711_Calibrate(1000.0f);
  HAL_Delay(100);
  
  // 3. 再次去皮
  while (!HX711_TareTimeout(5000))
  {
      OLED_Clear();
      OLED_ShowString(2, 1, "SCALE");
      OLED_ShowString(3, 1, "NOT CONNECT");
      HAL_Delay(500);
  }
  HAL_TIM_Base_Start_IT(&htim3);
  // W25Q64_SectorErase(16);
  OLED_Clear();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    Rawval = HX711_Read();
    Weight = HX711_GetWeight(Rawval);
    Serial_Printf("weight:%.2f\r\n", Weight);
    
    if (current_display_state == DISPLAY_WEIGHING)
    {
        //显示重量
        // OLED_ShowCN(1, 1, 0, 1);
        // OLED_ShowCN(1, 2, 1, 1);
        DISPLAY_Zhong;
        DISPLAY_Liang;
        OLED_ShowString(1, 5, ":");
        OLED_ShowString(1, 16, "g");
        OLED_ShowFloat(1, 6, Weight, 5, 2);
        
        if (Weight >= 0.0f)
        {
            TotalPrice = Weight * UnitPrice/500.0f;
        }
        else
        {
            TotalPrice = 0.0f;
        }
        
        // OLED_ShowString(2, 1, "Price:");
        DISPLAY_Dan;
        DISPLAY_Jia;
        OLED_ShowString(2, 5, ":");
        OLED_ShowFloat(2, 6, UnitPrice, 3, 1);
        OLED_ShowString(2, 12, "/500g");
        // OLED_ShowString(3, 1, "Total:");
        DISPLAY_Zong;
        DISPLAY_Jiage;
        OLED_ShowString(3, 5, ":");
        OLED_ShowFloat(3, 6, TotalPrice, 3, 2);
        // 
        OLED_ShowString(3, 13, "yuan");
        // OLED_ShowString(3, 13, "￥");
        
        if (SaveResult != 0)
        {
            if (SaveResult == 1)
            {
                // OLED_ShowString(4, 1, "Save OK!     ");
              OLED_SHOW_SAVE_OK();
            }
            else if (SaveResult == 2)
            {
                OLED_ShowString(4, 1, "Weight < 0!  ");
            }
            else if (SaveResult == 3) 
            {
                OLED_ShowString(4, 1, "OverWeight!  ");
            }
            SaveDisplayTime = HAL_GetTick();
            SaveResult = 0;
        }
        
        if (SaveDisplayTime != 0 && (HAL_GetTick() - SaveDisplayTime) > 1000)
        {
            OLED_ShowString(4, 1, "             ");
            SaveDisplayTime = 0;
        }
    }
    else if (current_display_state == DISPLAY_HISTORY)
    {
        if (history_index == 0)
        {
            OLED_ShowString(1, 1, "No Record       ");
            OLED_ShowString(2, 1, "               ");
            OLED_ShowString(3, 1, "               ");
            OLED_ShowString(4, 1, "               ");
            HAL_Delay(100);
            continue;
        }
        if (save_index >= history_index)
        {
            save_index = history_index - 1;
        }

        HistoryRecord *record = Key_GetHistoryRecord(save_index);
        
        // OLED_ShowString(1, 1, "Weight:");
        DISPLAY_Zhong;
        DISPLAY_Liang;
        OLED_ShowString(1, 5, ":");
        OLED_ShowFloat(1, 6, record->weight, 5, 2);
        OLED_ShowString(1, 16, "g");
        DISPLAY_Dan;
        DISPLAY_Jia;
        // OLED_ShowString(2, 1, "Price:");
        OLED_ShowString(2, 5, ":");
        OLED_ShowFloat(2, 6, record->unit_price, 3, 1);
        OLED_ShowString(2, 12, "/500g");
        DISPLAY_Zong;
        DISPLAY_Jiage;
        // OLED_ShowString(3, 1, "Total:");
        OLED_ShowString(3, 5, ":");
        OLED_ShowFloat(3, 6, record->total_price, 3, 2);
        // DISPLAY_Yuan;
        OLED_ShowString(3, 13, "yuan");
        OLED_ShowString(4, 1, "Index:");
        // OLED_ShowFloat(4, 7, record->history_index + 1, 2, 0);
        OLED_ShowNum(4, 7, record->history_index + 1, 2);
        
        HAL_Delay(100);
    }
    
    HAL_Delay(100);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == (&htim3))
    {
        Key1_scan(Key1);
        Key2_scan(Key2);
        Key3_scan(Key3);
        Key4_scan(Key4);
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
