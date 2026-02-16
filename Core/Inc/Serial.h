#ifndef _SERIAL_H
#define _SERIAL_H

#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "stdio.h"
#include "string.h"
#include <stdarg.h>

void Serial_SendByte(uint8_t* Byte);
void Serial_SendArray(uint8_t *Array, uint16_t length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Num, uint16_t Length);
void Serial_Printf(char *format,...);

#endif
