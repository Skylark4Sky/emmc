/*
* _COPYRIGHT_
*
* File Name: gisunlink_lcd.c
* System Environment: JOHAN-PC
* Created Time:2019-05-24
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GUI.h"
#include "Lcd_Driver.h"
#include "gisunlink_lcd.h"
#include "gisunlink_timer.h"
#include "gisunlink_network_module.h"
#include "gisunlink_delay.h"

#define DEV_SN_PREFIX "设备识别码:"
#define DEV_VER_PREFIX "设备版本号:"

#define LCD_NET_IDLE 								"状态:空闲中"
#define LCD_NET_START_CONNECT				"状态:开始连接"
#define LCD_NET_CONNECTING					"状态:网络连接中"
#define LCD_NET_CONNECTED						"状态:网络已连接"
#define LCD_NET_DISCONNECTED				"状态:已断开网络"
#define LCD_NET_RECONNECTING				"状态:网络重连中"
#define LCD_NET_ENT_CONFIG					"状态:进入配对"
#define LCD_NET_EXI_CONFIG					"状态:退出配对"
#define LCD_NET_SAVE_CONFIG					"状态:保存配对"
#define LCD_NET_TIME_SUCCEED				"状态:时钟同步成功"
#define LCD_NET_TIME_FAILED					"状态:时钟同步中"

#define LCD_CONNECTED_SERVICE				"状态:已连上平台"
#define LCD_DISCONNECTED_SERVICE		"状态:已断开平台"

#define LCD_REGISTER_GSM_SUCCEED		"状态:GSM注册成功"
#define LCD_REGISTER_GSM_FAILED			"状态:GSM注册失败"

#define MAX_STRING_TASK 4
#define MAX_DIGIT_TASK 10

typedef enum {
	REFRESH_DIGIT,
	REFRESH_STRING,
} REFRESH_TYPE;

typedef enum {
	REFRESH_DISENABLE,	
	REFRESH_ENABLE,
} REFRESH_STATE;

typedef struct _gisunlink_coor {
	uint8_t x;
	uint8_t y;
} gisunlink_coor;

typedef struct _gisunlink_refresh_digit {
	uint8_t refresh;
	gisunlink_coor coor;	//坐标
	uint16_t color;
} gisunlink_refresh_digit;

typedef struct _gisunlink_refresh_string {
	uint8_t refresh;
	gisunlink_coor coor;	//坐标
	uint16_t color;	
	uint8_t buffer[27];
} gisunlink_refresh_string;

gisunlink_refresh_digit refresh_digit[MAX_DIGIT_TASK];
gisunlink_refresh_string refresh_string[MAX_STRING_TASK];

static void gisunlink_lcd_init_ui(void) {
	Gui_DrawFont_GBK16(5,10,DGRAY,BLACK,(u8 *)"信号:");	
	Gui_DrawFont_GBK16(45,10,DGRAY,BLACK,(u8 *)"00");	
	Gui_DrawFont_GBK16(130,10,DGRAY,BLACK,(u8 *)"状态:空闲中");
	//顶部横线
	Gui_DrawLine(0,35,220,35,MAROON); 	//YELLOW
	//底部横线
	Gui_DrawLine(0,125,220,125,MAROON);
	//Gui_DrawFont_GBK16(46,134,TEAL,BLACK,(u8 *)"设备版本号:000000");
	Gui_DrawFont_GBK16(6,134,TEAL,BLACK,(u8 *)"设备版本号:GSL000000000000");	
	Gui_DrawFont_GBK16(6,154,TEAL,BLACK,(u8 *)"设备识别码:000000000000000");		
}

void gisunlink_lcd_init(void) {

	Lcd_Init();	
	Lcd_Clear(BLACK);

	gisunlink_lcd_init_ui();
	LCD_LED_SET;	
	refresh_string[0].refresh = REFRESH_DISENABLE;
	refresh_string[0].color = DGRAY;
	refresh_string[0].coor.x = 45; refresh_string[0].coor.y = 10; 

	refresh_string[1].refresh = REFRESH_DISENABLE;
	refresh_string[1].color = DGRAY;
	refresh_string[1].coor.x = 130; refresh_string[1].coor.y = 10; 

	refresh_string[2].refresh = REFRESH_DISENABLE;
	refresh_string[2].color = TEAL;	
	refresh_string[2].coor.x = 6; refresh_string[2].coor.y = 134; 

	refresh_string[3].refresh = REFRESH_DISENABLE;
	refresh_string[3].color = TEAL;	
	refresh_string[3].coor.x = 6; refresh_string[3].coor.y = 154; 	

	refresh_digit[0].refresh = REFRESH_DISENABLE;
	refresh_digit[0].coor.x = 15; refresh_digit[0].coor.y = 45; 

	refresh_digit[1].refresh = REFRESH_DISENABLE;
	refresh_digit[1].coor.x = 55; refresh_digit[1].coor.y = 45; 

	refresh_digit[2].refresh = REFRESH_DISENABLE;
	refresh_digit[2].coor.x = 95; refresh_digit[2].coor.y = 45; 

	refresh_digit[3].refresh = REFRESH_DISENABLE;
	refresh_digit[3].coor.x = 135; refresh_digit[3].coor.y = 45; 	

	refresh_digit[4].refresh = REFRESH_DISENABLE;
	refresh_digit[4].coor.x = 175; refresh_digit[4].coor.y = 45; 	

	refresh_digit[5].refresh = REFRESH_DISENABLE;
	refresh_digit[5].coor.x = 15; refresh_digit[5].coor.y = 85; 	

	refresh_digit[6].refresh = REFRESH_DISENABLE;
	refresh_digit[6].coor.x = 55; refresh_digit[6].coor.y = 85; 	

	refresh_digit[7].refresh = REFRESH_DISENABLE;
	refresh_digit[7].coor.x = 95; refresh_digit[7].coor.y = 85; 	

	refresh_digit[8].refresh = REFRESH_DISENABLE;
	refresh_digit[8].coor.x = 135; refresh_digit[8].coor.y = 85; 	

	refresh_digit[9].refresh = REFRESH_DISENABLE;
	refresh_digit[9].coor.x = 175; refresh_digit[9].coor.y = 85; 		
}

char *reverse(char *s) {
	char temp;
	char *p = s;
	char *q = s;
	while(*q) {
		++q;
	}

	q--;

	while(q > p){
		temp = *p;
		*p++ = *q;
		*q-- = temp;
	}
	return s;
}

char *gisunlink_itoa(int n) {
	int i = 0,isnegative = 0;
	static char s[8];
	memset(s,0x0,8);
	if((isnegative = n) < 0) { 
		n = -n;
	}

	do {
		s[i++] = n%10 + '0';
		n = n/10;
	}while(n > 0);

	if(isnegative < 0) {
		s[i++] = '-';
	}
	s[i] = '\0';
	return reverse(s);
}

char *gisunlink_calc_update_progress(uint32_t firmware_size, uint32_t transfer_offset, uint16_t burst) {
	float firmware_burst = ((firmware_size/burst)/100.0) * burst;
	uint16_t progress = 100 - ((firmware_size - transfer_offset)/firmware_burst);
	static char status_string[18] = {0};	
	snprintf(status_string,18,"%s%02d%s",LCD_STATE_UPDATEING,progress,"%");
	return status_string;
}

void gisunlink_lcd_set_apRssi(signed char apRssi) {
	const char *string = gisunlink_itoa(apRssi);
	gisunlink_refresh_string *string_task = NULL;	
	uint8_t string_len = strlen(string);
	uint8_t Rssi[5] = {0x20,0x20,0x20,0x20,0x00};
	string_task = &refresh_string[RSSI_STRING];	
	memcpy(Rssi,string,string_len);
	memcpy(string_task->buffer,Rssi,5);
	string_task->coor.x = 45;string_task->coor.y = 10;
	string_task->refresh = REFRESH_ENABLE;
}

void gisunlink_lcd_set_status(const char *string) {
	uint8_t string_len = 0;
	uint8_t left_offset = 17;
	gisunlink_refresh_string *string_task = NULL;		
	uint8_t status[18] = {0x20,0x20,0x20,0x20,0x20,
		0x20,0x20,0x20,0x20,0x20,
		0x20,0x20,0x20,0x20,0x20,
		0x20,0x20,0x00};	
	if(string) {
		string_len = strlen(string);
		string_task = &refresh_string[STATUS_STRING];
		if(string_len <= left_offset) {
			left_offset -= string_len;
			string_task->coor.x = 80;string_task->coor.y = 10;			
			memcpy(status + left_offset,string,string_len);
			memcpy(string_task->buffer,status,18);
			string_task->refresh = REFRESH_ENABLE;			
		}
	}
}

void gisunlink_lcd_set_bottom(const char *string, uint8_t type) {
	uint8_t string_len = 0;	
	uint8_t prefix_len = 0;	
	gisunlink_refresh_string *string_task = NULL;	

	uint8_t string_buf[27] = {0x20,0x20,0x20,0x20,0x20,
		0x20,0x20,0x20,0x20,0x20,
		0x20,0x20,0x20,0x20,0x20,
		0x20,0x20,0x20,0x20,0x20,
		0x20,0x20,0x20,0x20,0x20,		
		0x20,0x00};	

	if(string == NULL) {
		return;
	}	

	switch(type) {
		case VERSION_STRING:
			prefix_len = strlen(DEV_VER_PREFIX);
			memcpy(string_buf,DEV_VER_PREFIX,prefix_len);			
			string_task = &refresh_string[VERSION_STRING];	
			break;
		case SN_STRING:
			prefix_len = strlen(DEV_SN_PREFIX);		
			memcpy(string_buf,DEV_SN_PREFIX,prefix_len);		
			string_task = &refresh_string[SN_STRING];
			break;
	}
	
	if(string && string_task) {
		string_len = strlen(string);
		if((string_len + prefix_len) <= 27) {
			memcpy(string_buf + prefix_len,string,string_len);	
			memcpy(string_task->buffer,string_buf,sizeof(string_buf));		
			string_task->refresh = REFRESH_ENABLE;
		}		
	}
}

void gisunlink_lcd_set_port_status(uint8_t id,uint8_t status) {
	if(id < MAX_COM_QUANTITY && status <= PORT_BREAKDOWN) {
		gisunlink_refresh_digit *task = &refresh_digit[id];
		uint16_t color = GREEN;
		switch(status) {
			case PORT_BREAKDOWN:
				color = GRAY2;
				break;			
			case PORT_CHARGEING:
				color = RED;
				break;
			case PORT_LEISURE:
				color = DARK_GREEN;
				break;		
		}
		if(color != task->color) {
			task->refresh = REFRESH_ENABLE;
			task->color = color;		
		}
	}
}

void gisunlink_lcd_refresh_network_state(uint8_t state) {
	switch(state) 
	{
		case GISUNLINK_NETMANAGER_IDLE:              //空闲
			gisunlink_lcd_set_status(LCD_NET_IDLE);
			break;						
		case GISUNLINK_NETMANAGER_START:             //开始连接
			gisunlink_lcd_set_status(LCD_NET_START_CONNECT);
			break;
		case GISUNLINK_NETMANAGER_CONNECTING:        //连接中
			gisunlink_lcd_set_status(LCD_NET_CONNECTING);
			break;
		case GISUNLINK_NETMANAGER_CONNECTED:         //已连接
			gisunlink_lcd_set_status(LCD_NET_CONNECTED);
			break;
		case GISUNLINK_NETMANAGER_DISCONNECTED:      //断开连接
			gisunlink_lcd_set_status(LCD_NET_DISCONNECTED);
			break;						
		case GISUNLINK_NETMANAGER_RECONNECTING:      //重连中
			gisunlink_lcd_set_status(LCD_NET_RECONNECTING);
			break;						
		case GISUNLINK_NETMANAGER_ENT_CONFIG:        //进入配对
			gisunlink_lcd_set_status(LCD_NET_ENT_CONFIG);
			break;						
		case GISUNLINK_NETMANAGER_EXI_CONFIG:        //退出配对
			gisunlink_lcd_set_status(LCD_NET_EXI_CONFIG);
			break;						
		case GISUNLINK_NETMANAGER_SAVE_CONFIG:       //保存配对信息
			gisunlink_lcd_set_status(LCD_NET_SAVE_CONFIG);
			break;						
		case GISUNLINK_NETMANAGER_TIME_SUCCEED:      //同步时钟成功
			gisunlink_lcd_set_status(LCD_NET_TIME_SUCCEED);
			break;					
		case GISUNLINK_NETMANAGER_TIME_FAILED:       //同步时钟失败
			gisunlink_lcd_set_status(LCD_NET_TIME_FAILED);						
			break;						
		case GISUNLINK_NETMANAGER_CONNECTED_SER:			//已连上平
			gisunlink_lcd_set_status(LCD_CONNECTED_SERVICE);
			break;
		case GISUNLINK_NETMANAGER_DISCONNECTED_SER:	//断开平台连接
			gisunlink_lcd_set_status(LCD_DISCONNECTED_SERVICE);
			break;
		case GISUNLINK_NETMANAGER_GSM_CONNECTED:			//GSM注册成功
			gisunlink_lcd_set_status(LCD_REGISTER_GSM_SUCCEED);
			break;
		case GISUNLINK_NETMANAGER_GSM_DISCONNECTED:	//GSM注册失败						
			gisunlink_lcd_set_status(LCD_REGISTER_GSM_FAILED);
			break;
		default :
			break;
	}	
}

void gisunlink_lcd_refresh(void) {
	uint8_t index = 0;

	for(index = 0; index < MAX_DIGIT_TASK; index++) {
		gisunlink_refresh_digit *digit_task = &refresh_digit[index];
		if(digit_task->refresh == REFRESH_ENABLE) {
			digit_task->refresh = REFRESH_DISENABLE;
			Gui_DrawFont_Num32(digit_task->coor.x,digit_task->coor.y,digit_task->color,BLACK,index);
			DelayMs(10);			
		}
	}

	for(index = 0; index < MAX_STRING_TASK; index++) {
		gisunlink_refresh_string *string_task = &refresh_string[index];
		if(string_task->refresh == REFRESH_ENABLE) {
			string_task->refresh = REFRESH_DISENABLE;
			Gui_DrawFont_GBK16(string_task->coor.x,string_task->coor.y,string_task->color,BLACK,string_task->buffer);
			DelayMs(10);
		}
	}
}

