/*
* _COPYRIGHT_
*
* File Name:gisunlink_network_task.h
* System Environment: JOHAN-PC
* Created Time:2019-08-13
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#ifndef __GISUNLINK_NETWORK_TASK_H__
#define __GISUNLINK_NETWORK_TASK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "gisunlink.h"

void gisunlink_dev_fw_info(gisunlink *global, gisunlink_recv_frame *frame); 

void gisunlink_dev_fw_trans(gisunlink *global, gisunlink_recv_frame *frame);

void gisunlink_dev_fw_ready(gisunlink *global, gisunlink_recv_frame *frame);

void gisunlink_task_control(gisunlink *global, gisunlink_recv_frame *frame);

#ifdef __cplusplus
}
#endif

#endif //__GISUNLINK_NETWORK_TASK_H__
