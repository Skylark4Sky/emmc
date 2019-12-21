/*
* _COPYRIGHT_
*
* File Name:gisunlink_conf.h
* System Environment: JOHAN-PC
* Created Time:2019-08-13
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#ifndef __GISUNLINK_CONF_H__
#define __GISUNLINK_CONF_H__

#include "gisunlink.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
	SYSTEM_GSM_MODE = 0xFF,
	SYSTEM_WIFI_MODE = 0x55,
} SYSTEM_RUM_MODE;

typedef enum {
	SYSTEM_UNINIT = 0xFF,	
	SYSTEM_INIT = 0x88,	
} SYSTEM_CONFIG;

uint8_t gisunlink_conf_init(gisunlink *global);

uint8_t gisunlink_get_system_conf(gisunlink *global);

uint8_t gisunlink_set_system_conf(gisunlink *global);

void gisunlink_get_comList_conf(gisunlink *global);

void gisunlink_set_comList_conf(gisunlink *global); 

#ifdef __cplusplus
}
#endif

#endif //__GISUNLINK_CONF_H__
