/*
* _COPYRIGHT_
*
* File Name: gisunlink_conf.c
* System Environment: JOHAN-PC
* Created Time:2019-08-13
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#include <stdio.h>
#include <string.h>
#include "gisunlink.h"
#include "gisunlink_conf.h"
#include "gisunlink_delay.h"
#include "gisunlink_eeprom.h"

uint8_t gisunlink_conf_init(gisunlink *global) {
	uint8_t ret = SYSTEM_UNINIT;
		if(ee_ReadBytes((uint8_t *)&global->conf, SYSTEM_STATE_ADDRESS, sizeof(gisunlink_conf)) == 0) {
		//	if(global->conf.init != SYSTEM_INIT || global->conf.init == SYSTEM_UNINIT) {
			//如果init 不等于88 初始化配置
			if(global->conf.init != SYSTEM_INIT) {				
				memset((void *)&global->conf,0x00,sizeof(gisunlink_conf));
				ret = global->conf.init = SYSTEM_INIT;
				global->conf.mode = SYSTEM_GSM_MODE;
				DelayMs(10);
				ee_WriteBytes((uint8_t *)&global->conf, SYSTEM_STATE_ADDRESS, sizeof(gisunlink_conf));	
				DelayMs(10);
				ee_ReadBytes((uint8_t *)&global->conf, SYSTEM_STATE_ADDRESS, sizeof(gisunlink_conf));
			} else {
				ret = SYSTEM_INIT;
			}
		} else {
			ret = global->conf.init = SYSTEM_UNINIT;
		}
		return ret;		
}

uint8_t gisunlink_get_system_conf(gisunlink *global) {
	uint8_t ret = SYSTEM_UNINIT;	
	if(global) {
		if(ee_ReadBytes((uint8_t *)&global->conf, SYSTEM_STATE_ADDRESS, sizeof(gisunlink_conf)) == 0) {
			ret = global->conf.init;
		} 
		return ret;
	}
	return ret;
}

uint8_t gisunlink_set_system_conf(gisunlink *global) {
	if(global) {
		if(global->conf.init == SYSTEM_INIT) {
			ee_WriteBytes((uint8_t *)&global->conf, SYSTEM_STATE_ADDRESS, sizeof(gisunlink_conf));
		}		
		return global->conf.init;
	}
	return SYSTEM_UNINIT;
}

void gisunlink_get_comList_conf(gisunlink *global) {
	uint8_t comID = 0;
	if(global != NULL) 
		while(comID < MAX_COM_QUANTITY) {
			ee_ReadBytes((uint8_t *)&global->comList[comID], DEVICE_COM_ADDRESS * comID, sizeof(charge_com));
			global->energyOffset[comID] = global->comList[comID].used;//读出之前的使用值
			global->comList[comID++].used = 0;			
		}	
	
}

void gisunlink_set_comList_conf(gisunlink *global) {
	uint8_t comID = 0;
	if(global != NULL) {
		while(comID < MAX_COM_QUANTITY) {
			global->comList[comID].used += global->energyOffset[comID];
			ee_WriteBytes((uint8_t *)&global->comList[comID], DEVICE_COM_ADDRESS * comID, sizeof(charge_com));	
			comID++;
		}
	}
} 

