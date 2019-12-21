/*
* _COPYRIGHT_
*
* File Name:gisunlink_key.h
* System Environment: JOHAN-PC
* Created Time:2019-05-16
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#ifndef __GISUNLINK_KEY_H__
#define __GISUNLINK_KEY_H__

#include "stm32f10x.h"
#include "gisunlink.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define NET_KEY       	0x01
#define MOED_KEY       	0x02
#define BAK_KEY       	0x04

#define KEY_SHORT       0x40
#define KEY_LONG       	0x80
#define KEY_NONE       	0x00

/*! @brief 初始化按键 
 * @return void
 */
void gisunlink_key_init(void);

/*! @brief 按键
 * @return void
 */
void gisunlink_key_system_scan(void);

/*! @brief 按键扫描
 * @return void
 */
void gisunlink_key_scan(gisunlink *global);

#ifdef __cplusplus
}
#endif
#endif //__GISUNLINK_KEY_H__

