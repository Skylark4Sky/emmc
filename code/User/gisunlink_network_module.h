/*
* _COPYRIGHT_
*
* File Name:gisunlink_network_module.h
* System Environment: JOHAN-PC
* Created Time:2019-07-02
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#ifndef __GISUNLINK_NETWORK_MODULE_H__
#define __GISUNLINK_NETWORK_MODULE_H__

#include "stm32f10x.h"
#include "gisunlink.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define BYTES_TO_STRUCT(type,target,cmd,data,data_len) type target; gisunlink_bytes_to_struct(cmd,&target,data,data_len);
	
typedef enum {
	USE_USART1,
	USE_USART2,
	USE_USART3
} USART_ID;	

/*! @brief 无线模组初始化 
 * @param USART_ID 
 * @return uint8_t
 */
uint8_t gisunlink_network_module_init(USART_ID UsartID);

/*! @brief 发送数据到无线模组 
 * @param GISUNLINK_MODULE_CMD 
 * @param buffer 
 * @param size 
 * @return uint16_t 发送长度
 */
uint16_t gisunlink_network_module_write(uint8_t cmd,uint8_t *buffer, uint16_t size);

/*! @brief 回复数据到无线模组 
 * @param 流控ID 
 * @param GISUNLINK_MODULE_CMD 
 * @param buffer 
 * @param size 
 * @return uint16_t 发送长度
 */
uint16_t gisunlink_network_module_respond(uint32_t id,uint8_t cmd,uint8_t *buffer, uint16_t size);

/*! @brief 结构转为字符串 
 * @param cmd 
 * @param subcmd 
 * @param struct 
 * @param mode 
 * @return gisunlink_frame 
 */
gisunlink_frame *gisunlink_struct_to_bytes(uint8_t cmd,uint8_t subcmd, void *structptr);

/*! @brief 字符串转为结构
 * @param cmd 
 * @param structure  
 * @param buffer 
 * @param size 
 * @return uint8_t 
 */
uint8_t gisunlink_bytes_to_struct(uint8_t cmd,void *structure,uint8_t *buffer, uint16_t size);

/*! @brief 取数据从无线模组 
 * @param GISUNLINK_MESSAGE_CALLBACK 
 * @return void
 */
void gisunlink_network_module_read(void(*msgCb)(gisunlink_recv_frame *frame));

/*! @brief 提交单个节点数据到服务器 
 * @param gisunlink 端口集合 
 * @param comID 端口ID 
 * @param behavior 命令 
 * @return void
 */
void gisunlink_network_single_com_post(gisunlink *global, uint8_t comID, uint8_t behavior);

/*! @brief 提交全部节点数据到服务器 
 * @param gisunlink 端口集合 
 * @param behavior 命令 
 * @return void
 */
void gisunlink_network_multi_com_post(gisunlink *global, uint8_t behavior);

/*! @brief 无线模组通信中断处理 
 * @param void 
 * @return void
 */
void gisunlink_network_module_irq(void);

#ifdef __cplusplus
}
#endif
#endif //__GISUNLINK_NETWORK_MODULE_H__
