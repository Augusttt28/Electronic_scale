#include "hx711.h"
#include "stm32f1xx_hal.h"

static KalmanFilter kf;
static float scale_factor = 215.73f;
static int32_t tare_offset = 130;

//HX711初始化
void HX711_Init(void)
{
    HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_RESET);
}

void HX711_KalmanInit(float q, float r, float initial_value)
{
    kf.q = q;
    kf.r = r;
    kf.x = initial_value;
    kf.p = 1.0f;
    kf.k = 0.0f;
}

/** 
    * @brief 读取HX711的原始数据
    * 返回值：24位有符号整数
    */
int32_t HX711_Read(void)
{
    int32_t value = 0;
    uint8_t i;

    while (HAL_GPIO_ReadPin(HX711_OUT_GPIO_Port, HX711_OUT_Pin) != GPIO_PIN_RESET);

    for (i = 0; i < 24; i++)
    {
        HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_SET);
        value <<= 1;
        HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_RESET);
        if (HAL_GPIO_ReadPin(HX711_OUT_GPIO_Port, HX711_OUT_Pin))
        {
            value++;
        }
    }

    HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_RESET);

    if (value & 0x800000)
    {
        value |= 0xFF000000;
    }

    return value;
}

/** 
    * @brief HX711进入低功耗模式
    */
void HX711_PowerDown(void)
{
    HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_SET);
}

void HX711_PowerUp(void)
{
    HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_RESET);
}

//去皮函数，计算平均值作为去皮偏移量
void HX711_Tare(void)
{
    int32_t sum = 0;
    for (uint8_t i = 0; i < 10; i++)
    {
        sum += HX711_Read();
    }
    tare_offset = sum / 10;
}

void HX711_Calibrate(float known_weight)
{
    int32_t sum = 0;
    for (uint8_t i = 0; i < 10; i++)
    {
        sum += HX711_Read();
    }
    int32_t average = sum / 10;
    scale_factor = (float)(average - tare_offset) / known_weight;
}

float HX711_GetWeight(int32_t raw_value)
{
    float weight = (float)(raw_value - tare_offset) / scale_factor;
    return HX711_KalmanFilter(weight);
}

float HX711_KalmanFilter(float measurement)
{
    kf.p = kf.p + kf.q;
    kf.k = kf.p / (kf.p + kf.r);
    kf.x = kf.x + kf.k * (measurement - kf.x);
    kf.p = (1.0f - kf.k) * kf.p;
    return kf.x;
}
