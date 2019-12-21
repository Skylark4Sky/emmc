/*
* _COPYRIGHT_
*
* File Name:gisunlink_system.h
* System Environment: JOHAN-PC
* Created Time:2019-08-13
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#ifndef __GISUNLINK_SYSTEM_H__
#define __GISUNLINK_SYSTEM_H__

#include "gisunlink.h"

#ifdef __cplusplus
extern "C"
{
#endif

gisunlink *gisunlink_system_init(void);

void gisunlink_system_network_message(gisunlink_recv_frame *frame);

uint8_t gisunlink_system_check_route_work(void); 

void gisunlink_system_switch_timer(uint8_t timer);

void gisunlink_system_tick_post_data(void); 

void gisunlink_system_lcd_refresh(void);

void gisunlink_system_chargr_task_check(void);

void gisunlink_system_time_tick(void);

uint32_t gisunlink_system_get_time_tick(void);

void gisunlink_system_powerdown(void);

void gisunlink_system_powerdown_backup(void);

void gisunlink_system_soft_reset(void);

#ifdef __cplusplus
}
#endif

#endif //__GISUNLINK_SYSTEM_H__
