/*
* _COPYRIGHT_
*
* File Name:gisunlink_lcd.h
* System Environment: JOHAN-PC
* Created Time:2019-05-24
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#ifndef __GISUNLINK_LCD_H__
#define __GISUNLINK_LCD_H__

#include "stm32f10x.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LCD_STATE_START_UPDATE		"状态:开始升级"
#define LCD_STATE_UPDATEING				"状态:升级中"
#define LCD_STATE_UPDATE_FAILED		"状态:升级失败"
#define LCD_STATE_UPDATE_SUCCEED  "状态:升级成功"
#define LCD_STATE_UPDATE_TIMEOUT 	"状态:升级超时"
#define LCD_STATE_EXIT_UPDATE 		"状态:退出升级"

typedef enum {
	PORT_CHARGEING = 0x0,	//充电中
	PORT_LEISURE = 0x1,		//空闲中
	PORT_BREAKDOWN = 0x2,	//故障中
} PORT_STATUS;

typedef enum {
	RSSI_STRING,	
	STATUS_STRING,
	VERSION_STRING,	
	SN_STRING,		
} STRING_TYPE;

void gisunlink_lcd_init(void);

void gisunlink_lcd_set_apRssi(signed char apRssi);

void gisunlink_lcd_set_status(const char *string);

void gisunlink_lcd_set_bottom(const char *string, uint8_t type);

char *gisunlink_itoa(int n);
	
char *gisunlink_calc_update_progress(uint32_t firmware_size, uint32_t transfer_offset, uint16_t burst);

void gisunlink_lcd_set_port_status(uint8_t id,uint8_t status);

void gisunlink_lcd_refresh_network_state(uint8_t state);

void gisunlink_lcd_refresh(void);

#ifdef __cplusplus
}
#endif

#endif //__GISUNLINK_LCD_H__
