#ifndef __HX711_H
#define __HX711_H


#include "main.h"

typedef struct {
    float q;
    float r;
    float x;
    float p;
    float k;
} KalmanFilter;

void HX711_Init(void);
void HX711_KalmanInit(float q, float r, float initial_value);
int32_t HX711_Read(void);
void HX711_PowerDown(void);
void HX711_PowerUp(void);
void HX711_Tare(void);
void HX711_Calibrate(float known_weight);
float HX711_GetWeight(int32_t raw_value);
float HX711_KalmanFilter(float measurement);

#endif
