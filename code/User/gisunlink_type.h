/*
* _COPYRIGHT_
*
* File Name:gisunlink_type.h
* System Environment: JOHAN-PC
* Created Time:2019-08-13
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#ifndef __GISUNLINK_TYPE_H__
#define __GISUNLINK_TYPE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define USART_DEBUG
#define EM_DEBUG

#define MAX_COM_QUANTITY			10
#define STM32_UNIQUEID_SIZE			12

#define CHARGR_OVER_CURRENT			50			//空载或充满条件
#define CHK_CURRENT_MA				5

#define SUCCEED						0	//成功
#define FAILED						1	//失败

#define TURE						1	//真
#define FALSE						0	//假

#define TIMER_10SECOND				15 //充电中提交频率
#define TIMER_60SECOND				60 //空闲中提交频率

#define IS_BREAKDOWN				1	//故障
#define NO_BREAKDOWN				0	//没故障

#define IS_ENABLE					1	//使用
#define NO_ENABLE					0	//没使用

#define IS_FALLING					1	//下降
#define NO_FALLING					0

#define SINGLE_RECORD				1 	//单条记录
#define MULTI_RECORD				MAX_COM_QUANTITY	//多条记录

#define FIRMWARE_MD5_SIZE			33 //md5值长度

#define NO_LOAD_TIME				30 //默认空载时间

#define OPEN_PORT_FAILED				0x02			//打开通道失败 
#define READ_PORT_FAILED				0x04			//读通道失败 
#define VOLTAGE_OVERFLOW				0x08			//电压不在范围 
#define CURREN_OVERFLOW_3A			0x10			//电流超过3A
#define CURREN_OVERFLOW_SETTING		0x20			//电流超过设置的值 

#define USART_RX_BUFFER_SIZE 		280
#define USART_TX_BUFFER_SIZE 		512
#define USART_BUF_DIMENSION  		12

#ifdef __cplusplus
}
#endif

#endif //__GISUNLINK_TYPE_H__
