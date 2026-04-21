#include "MySPI.h"
#include "W25Q64_Ins.h"
#include "stm32f1xx_hal.h"
#include <string.h>

extern uint32_t SaveCount;
extern uint32_t history_index;

void W25Q64_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count);

/** 每条记录 16 字节，与 Key.c 中 PageProgram(SaveCount*16) 一致 */
#define W25Q64_RECORD_SIZE 16u

static uint8_t W25Q64_RecordSlotEmpty(const uint8_t *buf)
{
	uint16_t i;
	for (i = 0; i < W25Q64_RECORD_SIZE; i++)
	{
		if (buf[i] != 0xFFu)
		{
			return 0;
		}
	}
	return 1;
}

/** 第一个全 0xFF 的槽下标 = 已写入条数（连续从地址 0 存放时） */
static uint32_t W25Q64_FindFirstEmptyRecordSlot(void)
{
	uint8_t buf[16];
	const uint32_t max_slots = (8u * 1024u * 1024u) / W25Q64_RECORD_SIZE;
	uint32_t lo = 0;
	uint32_t hi = max_slots;

	while (lo < hi)
	{
		uint32_t mid = lo + (hi - lo) / 2u;
		W25Q64_ReadData(mid * W25Q64_RECORD_SIZE, buf, W25Q64_RECORD_SIZE);
		if (W25Q64_RecordSlotEmpty(buf))
		{
			hi = mid;
		}
		else
		{
			lo = mid + 1u;
		}
	}
	return lo;
}

/**
 * 从 Flash 恢复 SaveCount / history_index（与 Key_GetHistoryRecord 解析 layout 一致）。
 * 条数由“第一个空槽”确定；最后一条 data[12..15] 为保存时写入的 history_index，RAM 中应为 其+1。
 */
void W25Q64_RestoreHistoryFromFlash(void)
{
	uint8_t data[16];
	uint32_t n;
	uint32_t stored_hist_idx;

	n = W25Q64_FindFirstEmptyRecordSlot();
	SaveCount = n;

	if (n == 0u)
	{
		history_index = 0u;
		return;
	}

	W25Q64_ReadData((n - 1u) * W25Q64_RECORD_SIZE, data, W25Q64_RECORD_SIZE);
	memcpy(&stored_hist_idx, &data[12], sizeof(stored_hist_idx));
	history_index = stored_hist_idx + 1u;
}

/**
  * 函    数：W25Q64初始化
  * 参    数：无
  * 返 回 值：无
  */
void W25Q64_Init(void)
{
	MySPI_Init();					//先初始化底层的SPI
}

/**
  * 函    数：W25Q64读取ID号
  * 参    数：MID 工厂ID，使用输出参数的形式返回
  * 参    数：DID 设备ID，使用输出参数的形式返回
  * 返 回 值：无
  */
void W25Q64_ReadID(uint8_t *MID, uint16_t *DID)
{
	MySPI_Start();								//SPI起始
	MySPI_SwapByte(W25Q64_JEDEC_ID);			//交换发送读取ID的指令
	*MID = MySPI_SwapByte(W25Q64_DUMMY_BYTE);	//交换接收MID，通过输出参数返回
	*DID = MySPI_SwapByte(W25Q64_DUMMY_BYTE);	//交换接收DID高8位
	*DID <<= 8;									//高8位移到高位
	*DID |= MySPI_SwapByte(W25Q64_DUMMY_BYTE);	//或上交换接收DID的低8位，通过输出参数返回
	MySPI_Stop();								//SPI终止
}


/**
  * 函    数：W25Q64写使能
  * 参    数：无
  * 返 回 值：无
  */
void W25Q64_WriteEnable(void)
{
	MySPI_Start();								//SPI起始
	MySPI_SwapByte(W25Q64_WRITE_ENABLE);		//交换发送写使能的指令
	MySPI_Stop();								//SPI终止
}

/**
  * 函    数：W25Q64等待忙
  * 参    数：无
  * 返 回 值：无
  */
void W25Q64_WaitBusy(void)
{
	uint32_t Timeout;
	MySPI_Start();								//SPI起始
	MySPI_SwapByte(W25Q64_READ_STATUS_REGISTER_1);				//交换发送读状态寄存器1的指令
	Timeout = 100000;							//给定超时计数时间
	while ((MySPI_SwapByte(W25Q64_DUMMY_BYTE) & 0x01) == 0x01)	//循环等待忙标志位
	{
		Timeout --;								//等待时，计数值自减
		if (Timeout == 0)						//自减到0后，等待超时
		{
			/*超时的错误处理代码，可以添加到此处*/
			break;								//跳出等待，不等了
		}
	}
	MySPI_Stop();								//SPI终止
}

/**
  * 函    数：W25Q64页编程
  * 参    数：Address 页编程的起始地址，范围：0x000000~0x7FFFFF
  * 参    数：DataArray	用于写入数据的数组
  * 参    数：Count 要写入数据的数量，范围：0~256
  * 返 回 值：无
  * 注意事项：写入的地址范围不能跨页
  */
void W25Q64_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count)
{
	uint16_t i;
	
	W25Q64_WriteEnable();						//写使能
	
	MySPI_Start();								//SPI起始
	MySPI_SwapByte(W25Q64_PAGE_PROGRAM);		//交换发送页编程的指令
	MySPI_SwapByte(Address >> 16);				//交换发送地址23~16位
	MySPI_SwapByte(Address >> 8);				//交换发送地址15~8位
	MySPI_SwapByte(Address);					//交换发送地址7~0位
	for (i = 0; i < Count; i ++)				//循环Count次
	{
		MySPI_SwapByte(DataArray[i]);			//依次在起始地址后写入数据
	}
	MySPI_Stop();								//SPI终止
	
	W25Q64_WaitBusy();							//等待忙
}

/**
  * 函    数：W25Q64扇区擦除（4KB）
  * 参    数：Address 指定扇区的地址，范围：0x000000~0x7FFFFF
  * 返 回 值：无
  */
void W25Q64_SectorErase(uint32_t Address)
{
	W25Q64_WriteEnable();						//写使能
	
	MySPI_Start();								//SPI起始
	MySPI_SwapByte(W25Q64_SECTOR_ERASE_4KB);	//交换发送扇区擦除的指令
	MySPI_SwapByte(Address >> 16);				//交换发送地址23~16位
	MySPI_SwapByte(Address >> 8);				//交换发送地址15~8位
	MySPI_SwapByte(Address);					//交换发送地址7~0位
	MySPI_Stop();								//SPI终止
	
	W25Q64_WaitBusy();							//等待忙
}

/**
  * 函    数：W25Q64读取数据
  * 参    数：Address 读取数据的起始地址，范围：0x000000~0x7FFFFF
  * 参    数：DataArray 用于接收读取数据的数组，通过输出参数返回
  * 参    数：Count 要读取数据的数量，范围：0~0x800000
  * 返 回 值：无
  */
void W25Q64_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count)
{
	uint32_t i;
	MySPI_Start();								//SPI起始
	MySPI_SwapByte(W25Q64_READ_DATA);			//交换发送读取数据的指令
	MySPI_SwapByte(Address >> 16);				//交换发送地址23~16位
	MySPI_SwapByte(Address >> 8);				//交换发送地址15~8位
	MySPI_SwapByte(Address);					//交换发送地址7~0位
	for (i = 0; i < Count; i ++)				//循环Count次
	{
		DataArray[i] = MySPI_SwapByte(W25Q64_DUMMY_BYTE);	//依次在起始地址后读取数据
	}
	MySPI_Stop();								//SPI终止
}


/**
  * 函    数：W25Q64芯片擦除（64MB）
  * 参    数：无
  * 返 回 值：无
  */
void W25Q64_ChipErase(void)
{
	W25Q64_WriteEnable();						//写使能
	
	MySPI_Start();								//SPI起始
	MySPI_SwapByte(W25Q64_CHIP_ERASE);		//交换发送芯片擦除的指令
	MySPI_Stop();								//SPI终止
	
	W25Q64_WaitBusy();							//等待忙
}

/**
  * 函    数：W25Q64清空所有记录
  * 参    数：无
  * 返 回 值：无
  */
void W25Q64_ClearAllRecords(void)
{
	uint32_t i;
	uint32_t sector_count;
	
	if (SaveCount > 2048)
	{
		sector_count = 2048;
	}
	else
	{
		sector_count = SaveCount + 1;
	}
	
	for (i = 0; i < sector_count; i++)
	{
		W25Q64_SectorErase(i * 4096);
	}
}