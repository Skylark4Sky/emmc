/*
* _COPYRIGHT_
*
* File Name: gisunlink_key.c
* System Environment: JOHAN-PC
* Created Time:2019-07-02
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gisunlink.h"
#include "gisunlink_key.h"
#include "gisunlink_conf.h"
#include "gisunlink_delay.h"
#include "gisunlink_system.h"
#include "gisunlink_network_module.h"

#define KEY_RELEASE       					0
#define KEY_PRESS       						1
#define KEY_PRESS_CHK      					2
#define KEY_LONG_PRESS_CHK					3
#define KEY_LONG_PRESS_RELEASE			4

#define GISUNLINK_NET_KEY   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)
#define GISUNLINK_MOED_KEY  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)
#define GISUNLINK_BAK_KEY   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)

typedef struct _gisunlink_key {
	volatile uint8_t hasKeyPress;
} gisunlink_key, *Pgisunlink_key;

static gisunlink_key key = {0};

static void gisunlink_key_release(void) {
	key.hasKeyPress = KEY_NONE;
}

void gisunlink_key_init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;	

	gisunlink_key_release();
	SysTick_Config(SystemCoreClock/1000);	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB ,ENABLE);	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static uint8_t gisunlink_key_read(void) {	
	if(GISUNLINK_NET_KEY == 0) {
		return NET_KEY;
	}

	if(GISUNLINK_MOED_KEY == 0) {
		return MOED_KEY;  
	}

	if(GISUNLINK_BAK_KEY == 0) {
		return BAK_KEY;  
	}

	return KEY_NONE;
}

void gisunlink_get_scan(void) {
	static uint32_t LongPressTime = 0;
	static uint8_t state = KEY_RELEASE;
	static uint8_t LastPreeKey = 0, CurPressKey = 0;	
	CurPressKey = gisunlink_key_read();

	switch(state) {
		case KEY_RELEASE:
			if(CurPressKey != LastPreeKey) 
				state = KEY_PRESS;      //有按键按下
			break;  
		case KEY_PRESS:   
			if(CurPressKey == LastPreeKey) 
				state = KEY_PRESS_CHK;  //开始检测按键
			else 
				state = KEY_RELEASE;   //误触发
			break;   
		case KEY_PRESS_CHK: 
			if(CurPressKey == LastPreeKey) {//如果还是按着的状态 可能是长按
				state = KEY_LONG_PRESS_CHK;
			} else {            //短按
				state = KEY_RELEASE;         
				key.hasKeyPress = LastPreeKey|KEY_SHORT;  
			}
			break;    
		case KEY_LONG_PRESS_CHK: 
			if(CurPressKey == LastPreeKey) {       
				if(0 == LongPressTime) {
					LongPressTime = gisunlink_system_get_time_tick();
				}
				if(gisunlink_system_get_time_tick() >= (LongPressTime + 1500)) {//如果大于1.5秒 则为长按事件
					LongPressTime = 0;      
					state = KEY_LONG_PRESS_RELEASE;     
					key.hasKeyPress = LastPreeKey|KEY_LONG;
				} 
			} else {    
				LongPressTime = 0;     		 
				state = KEY_RELEASE;    
				key.hasKeyPress = LastPreeKey|KEY_SHORT;
			}
			break;  
		case KEY_LONG_PRESS_RELEASE:
			if(CurPressKey != LastPreeKey) {
				state = KEY_RELEASE;  
			}
#if 0
			if(CurPressKey == KEY_NONE && LastPreeKey == KEY_NONE) {
				state = KEY_RELEASE;   
			}
#endif

			break;
	}
	LastPreeKey = CurPressKey;	
}

void gisunlink_key_system_scan(void) {
	static uint8_t timer = 0;
	if(timer++ >= 20) {
		timer = 0;
		gisunlink_get_scan();
	}
}

static uint8_t gisunlink_get_key(void) {
	return key.hasKeyPress;
}

void gisunlink_key_scan(gisunlink *global) {	
	switch(gisunlink_get_key()) {
		case NET_KEY|KEY_SHORT:
			gisunlink_key_release();
			break;
		case MOED_KEY|KEY_SHORT:
			gisunlink_key_release();
			break;					
		case NET_KEY|KEY_LONG:
			gisunlink_key_release();
			if(global->conf.mode == SYSTEM_WIFI_MODE) {
				gisunlink_network_module_write(GISUNLINK_NETWORK_RESET,NULL,0);
			}
			break;			
		case MOED_KEY|KEY_LONG:
			gisunlink_key_release();			
			gisunlink_set_comList_conf(global);			
			if(global->conf.mode == SYSTEM_GSM_MODE) {
				global->conf.mode = SYSTEM_WIFI_MODE;
			} else if(global->conf.mode == SYSTEM_WIFI_MODE){
				global->conf.mode = SYSTEM_GSM_MODE;
			}				
			gisunlink_set_system_conf(global);		
			DelayMs(100);						
			gisunlink_system_soft_reset();				
			break;			
	}	
}

