/*
* _COPYRIGHT_
*
* File Name:gisunlink_coms.h
* System Environment: JOHAN-PC
* Created Time:2019-08-13
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#ifndef __GISUNLINK_COMS_H__
#define __GISUNLINK_COMS_H__

#include "gisunlink.h"

#ifdef __cplusplus
extern "C"
{
#endif

//复位端口
void gisunlink_com_reset(gisunlink *global, uint8_t comID, uint8_t clearAll);

//置位端口
void gisunlink_com_set(gisunlink *global, uint8_t comID, gisunlink_charge_task *charge_task);

//开始充电
uint8_t gisunlink_com_start_charge(gisunlink *global, uint8_t comID);

//停止充电
uint8_t gisunlink_com_stop_charge(gisunlink *global, uint8_t comID);

//端口检查
void gisunlink_chargring_chk(gisunlink *global,void(*NetworkPost)(gisunlink *global, uint8_t comID, uint8_t behavior));

#ifdef __cplusplus
}
#endif

#endif //__GISUNLINK_COMS_H__
