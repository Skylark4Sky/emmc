#ifndef __GISUNLINK_EEM_H
#define __GISUNLINK_EEM_H

#include "stm32f10x.h"
#include "gisunlink.h"

#define MAX_COMPACK_SIZE 32

//---start 串口通讯变量文件-------
typedef struct {
  u16 	RxLen;							//接收数据长度
  u16  	TxLen;
  u8 		*pTx; 
  u8 		RxBuf[MAX_COMPACK_SIZE];//接收缓存
  u8		TxBuf[MAX_COMPACK_SIZE];//发送缓存
} sComPack_TypeDef;
extern sComPack_TypeDef ComPack;

void HC595OutByte(uint16_t data);
void EMU_Calibration(void);
void gisunlink_energy_metering_Init(void);

uint8_t gisunlink_open_check_channel(uint8_t channel);
void gisunlink_close_check_channel(uint8_t channel);

void gisunlink_read_realtime_meter_data(gisunlink *global);

#endif
