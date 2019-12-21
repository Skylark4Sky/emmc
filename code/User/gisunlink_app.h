#ifndef __GISUNLINK_APP_H
#define __GISUNLINK_APP_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define USART_DEBUG
#define EM_DEBUG

#define CHARGR_OVER_CURRENT 50			//空载或充满条件
#define CHK_CURRENT_MA 			5

#define SUCCEED 		0	//成功
#define FAILED			1	//失败

#define TURE 				1	//真
#define FALSE				0	//假

#define TIMER_10SECOND	10 //充电中10秒提交一次
#define TIMER_60SECOND	60 //空闲中60秒提交一次

#define IS_BREAKDOWN 	1	//故障
#define NO_BREAKDOWN	0	//没故障

#define IS_ENABLE 		1	//使用
#define NO_ENABLE			0	//没使用

#define IS_FALLING      1	//下降
#define NO_FALLING      0

#define SINGLE_RECORD	1 	//单条记录
#define MULTI_RECORD	MAX_COM_QUANTITY	//多条记录

typedef enum {
	TIMER_UNKNOWN,
	TIMER_10S,
	TIMER_60S,
} TIMER_TYPE;

extern volatile uint8_t realTimeReadDateFlag;
extern volatile uint32_t energyOffset[10];

extern volatile uint8_t tim500msflag;
#endif


