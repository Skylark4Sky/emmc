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

#define LCD_STATE_START_UPDATE		"״̬:��ʼ����"
#define LCD_STATE_UPDATEING				"״̬:������"
#define LCD_STATE_UPDATE_FAILED		"״̬:����ʧ��"
#define LCD_STATE_UPDATE_SUCCEED  "״̬:�����ɹ�"
#define LCD_STATE_UPDATE_TIMEOUT 	"״̬:������ʱ"
#define LCD_STATE_EXIT_UPDATE 		"״̬:�˳�����"

typedef enum {
	PORT_CHARGEING = 0x0,	//�����
	PORT_LEISURE = 0x1,		//������
	PORT_BREAKDOWN = 0x2,	//������
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
