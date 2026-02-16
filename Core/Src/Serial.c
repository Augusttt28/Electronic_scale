#include "Serial.h"


//串口发送一个字节
void Serial_SendByte(uint8_t* Byte)
{
    HAL_UART_Transmit(&huart1, Byte, 1, HAL_MAX_DELAY);
}

//发送一个数组
void Serial_SendArray(uint8_t *Array, uint16_t length)
{
    HAL_UART_Transmit(&huart1, Array, length, HAL_MAX_DELAY);
}

//发送字符串
void Serial_SendString(char *String)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)String, strlen(String), HAL_MAX_DELAY);
}

//X的Y次方
uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

//发送数组
void Serial_SendNumber(uint32_t Num, uint16_t Length)
{
	uint16_t i;
	for(i = 0;i < Length;i++)
	{
		Serial_SendByte((uint8_t *)(Num/Serial_Pow(10, Length - i -1)%10 + '0'));
	}
}

/************************************************************************************************************
 * 函数名称: Serial_Printf
 * 函数功能: 使用串口，重定向printf
 * 输入参数: 字符串等
 * 输出参数: 无
 * 返回值: 无
************************************************************************************************************/
void Serial_Printf(char *format,...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	Serial_SendString(String);
}

