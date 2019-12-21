/*
* _COPYRIGHT_
*
* File Name: gisunlink_network_task.c
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
#include "gisunlink_lcd.h"
#include "gisunlink_type.h"
#include "gisunlink_coms.h"
#include "gisunlink_conf.h"
#include "gisunlink_delay.h"
#include "gisunlink_updata.h"
#include "gisunlink_system.h"
#include "gisunlink_network_task.h"
#include "gisunlink_network_module.h"

void gisunlink_dev_fw_info(gisunlink *global, gisunlink_recv_frame *frame) {
	uint8_t respond = GISUNLINK_DEVICE_TIMEOUT;

	if(frame->dir == GISUNLINK_COMM_REQ) { //带值回复
		global->system_update_flag = FALSE;							
		global->system.firmware.transfer_offset = 0xFFFF;

		if(frame->data_len) {
			BYTES_TO_STRUCT(gisunlink_device_firmware_info,firmware,frame->cmd,frame->data,frame->data_len);	
			
			if(global->system.route_work == IS_ENABLE) {							
				respond = GISUNLINK_DEVICE_TIMEOUT;//设备忙
			} else {									
				if(memcmp((const void *)global->conf.firmware_md5,firmware.md5,32) == 0) {
					respond = GISUNLINK_NO_NEED_UPGRADE;//不需要升级
				} else {
					gisunlink_set_comList_conf(global);						
					respond = GISUNLINK_NEED_UPGRADE; //需要升级
					global->system_update_flag = TRUE;												
					global->system.firmware.transfer_timeout = 0;
					global->system.firmware.transfer_offset = 0x00;
					global->system.firmware.info.size = firmware.size;
					memcpy((void *)(global->system.firmware.info.md5),firmware.md5,32);

					MD5Init(&global->system.firmware.md5);	
					gisunlink_lcd_set_status(LCD_STATE_START_UPDATE);
					gisunlink_lcd_refresh();
				}			
			}									
		}				
		gisunlink_network_module_respond(frame->id,frame->cmd,&respond,sizeof(uint8_t));
	}						
}

void gisunlink_dev_fw_trans(gisunlink *global, gisunlink_recv_frame *frame) {

	uint8_t respond_offset[2] = {0};
	if(frame->dir == GISUNLINK_COMM_REQ) { //带值回复
		BYTES_TO_STRUCT(gisunlink_device_firmware_transfer,transfer,frame->cmd,frame->data,frame->data_len);					

		respond_offset[0] = (transfer.offset & 0xFF);
		respond_offset[1] = ((transfer.offset >> 8) & 0xFF);

		if(global->system_update_flag == TRUE) {
			gisunlink_network_module_respond(frame->id,frame->cmd,respond_offset,sizeof(uint16_t));						
		} else {					
			respond_offset[0] = 0x00;
			respond_offset[1] = 0x00;
			global->system.firmware.transfer_offset = 0;
			gisunlink_network_module_respond(frame->id,frame->cmd,respond_offset,sizeof(uint16_t));
		}

		if(global->system_update_flag == TRUE && global->system.firmware.transfer_offset != transfer.offset) {
			if((transfer.offset - global->system.firmware.transfer_offset) == 1) {									
				if(transfer.size) {
					uint32_t data_offset = global->system.firmware.transfer_offset * 256;
					gisunlink_lcd_set_status(gisunlink_calc_update_progress(global->system.firmware.info.size,data_offset,256));
					gisunlink_lcd_refresh();															

					MD5Update(&global->system.firmware.md5, transfer.data, transfer.size);													
					if(gisunlink_write_flash_by_update(data_offset,transfer.data,transfer.size) == FAILED) {
						gisunlink_lcd_set_status(LCD_STATE_UPDATE_FAILED);
						gisunlink_lcd_refresh();
						global->system.firmware.transfer_timeout = 31;
					} else {
						global->system.firmware.transfer_timeout = 0;
						global->system.firmware.transfer_offset = transfer.offset;									
					}							
				}										
			}
		}
	}		
}

void gisunlink_dev_fw_ready(gisunlink *global, gisunlink_recv_frame *frame) {
	uint8_t firmware_chk = GISUNLINK_FIRMWARE_CHK_NO_OK;	
	uint8_t calc_md5[FIRMWARE_MD5_SIZE] = {0};
	uint8_t i = 0;
	uint8_t cmpOk = FAILED;

	if(frame->dir == GISUNLINK_COMM_REQ) { //带值回复
		if(global->system_update_flag == TRUE) {					
			global->system.firmware.transfer_offset = 0;
			MD5Final(global->system.firmware.md5_digest, &global->system.firmware.md5);

			for (i = 0; i < 16; ++i) {
				sprintf((char *)&(calc_md5[i * 2]), "%02x", global->system.firmware.md5_digest[i]);
			}

			if(strncmp((char *)calc_md5,(char *)global->system.firmware.info.md5,32) == 0) {
				firmware_chk = GISUNLINK_FIRMWARE_CHK_OK;								
				cmpOk = SUCCEED;
			}

			global->system.firmware.transfer_timeout = 0;

			gisunlink_network_module_respond(frame->id,frame->cmd,&firmware_chk,sizeof(uint8_t));

			if(cmpOk == SUCCEED) {				
				//global->conf.firmware_version = global->system.firmware.info.version;
				memcpy((void *)global->conf.firmware_md5,(void *)global->system.firmware.info.md5,32);

				gisunlink_set_system_conf(global);		
				gisunlink_lcd_set_status(LCD_STATE_UPDATE_SUCCEED);
				gisunlink_lcd_refresh();							

				DelayMs(2500);							
				gisunlink_enable_updata();						
				global->system_update_flag = FALSE;							
			} else {
				gisunlink_lcd_set_status(LCD_STATE_UPDATE_FAILED);
				gisunlink_lcd_refresh();
				global->system.firmware.transfer_timeout = 31;
			}
		} else {
			gisunlink_network_module_respond(frame->id,frame->cmd,&firmware_chk,sizeof(uint8_t));
		}
	}						
}

static uint8_t gisunlink_task_control_refresh_charge_task(charge_com *com,gisunlink_charge_task *charge_task) {
	uint8_t change = 0;

	if(com->token == charge_task->token) {
		if(com->total != charge_task->total) {
			change = 1;	
			com->total = charge_task->total;	
		}

		if(com->ptime != charge_task->ptime) {
			change = 1;
			com->ptime = charge_task->ptime;												
		}

		if(com->max_current != charge_task->max_current) {
			change = 1;	
			com->max_current = charge_task->max_current;												
		}
	}

	return change;
}

static void gisunlink_task_control_charge_task(gisunlink *global, uint8_t comID,gisunlink_charge_task *charge_task) {
	uint8_t respond_value = FAILED;
	if(global->comList[comID].breakdown == NO_BREAKDOWN) {
		if(global->comList[comID].enable == NO_ENABLE) { //端口未使用
			gisunlink_com_set(global,comID,charge_task);
			respond_value = gisunlink_com_start_charge(global,comID);

			if(respond_value == SUCCEED) {
				global->comList[comID].enable = IS_ENABLE;						//设置启动标志位		
				//提交端口开始充电状态到服务端
				gisunlink_lcd_set_port_status(comID,PORT_CHARGEING);
				global->comList[comID].cur_behavior = GISUNLINK_START_CHARGE;
				gisunlink_network_single_com_post(global,comID,GISUNLINK_START_CHARGE);											
			}	

			if(global->comList[comID].breakdown != NO_BREAKDOWN) {
				//提交端口故障状态到服务端
				gisunlink_lcd_set_port_status(charge_task->id,PORT_BREAKDOWN);
				global->comList[comID].cur_behavior = GISUNLINK_CHARGE_BREAKDOWN;		
				gisunlink_network_single_com_post(global,comID,GISUNLINK_CHARGE_BREAKDOWN);	
				//复位端口并且关闭端口
				gisunlink_com_reset(global,comID,0);
				gisunlink_com_stop_charge(global,comID);										
			}											

		} else { //端口已使用 
			if(gisunlink_task_control_refresh_charge_task(&global->comList[comID],charge_task)) {
				gisunlink_network_single_com_post(global,comID,GISUNLINK_COM_UPDATE);	
			} else {
				gisunlink_network_single_com_post(global,comID,GISUNLINK_COM_NO_UPDATE);	
			}											
		}
	} else { //端口故障
		global->comList[comID].cur_behavior = GISUNLINK_CHARGE_BREAKDOWN;				
		gisunlink_network_single_com_post(global,comID,GISUNLINK_CHARGE_BREAKDOWN);	
	}
}

void gisunlink_task_control(gisunlink *global, gisunlink_recv_frame *frame) {

	if(frame->dir == GISUNLINK_COMM_REQ) { 	
		//首字节为子命令
		switch((uint8_t)*frame->data) {
			case GISUNLINK_CHARGE_TASK://充电任务下发 
				{														
					uint8_t respond_value = FAILED;					
					BYTES_TO_STRUCT(gisunlink_charge_task,charge_task,frame->cmd,frame->data,frame->data_len);				
					//如果端口有效且没被使用
					if(charge_task.id < MAX_COM_QUANTITY) {
						if(global->system_update_flag == TRUE) { //系统升级中
							gisunlink_network_single_com_post(global,charge_task.id,GISUNLINK_UPDATE_FIRMWARE);
						} else {
							respond_value = SUCCEED;
							gisunlink_network_module_respond(frame->id,frame->cmd,&respond_value,sizeof(uint8_t));
							gisunlink_task_control_charge_task(global,charge_task.id,&charge_task);
						}
					} else {
						gisunlink_network_module_respond(frame->id,frame->cmd,&respond_value,sizeof(uint8_t));
					} 
				}
				break;
			case GISUNLINK_DEVIDE_STATUS://请求设备状态
				{
					uint8_t respond_value = SUCCEED;					
					uint8_t cmd = GISUNLINK_CHARGEING;
					BYTES_TO_STRUCT(gisunlink_device_status,device_status,frame->cmd,frame->data,frame->data_len);													
					//先插入回复信息
					gisunlink_network_module_respond(frame->id,frame->cmd,&respond_value,sizeof(uint8_t));							

					//获取单个端口状态
					if(device_status.id < MAX_COM_QUANTITY) {
						//提交端口状态到服务端
						if(global->comList[device_status.id].enable == NO_ENABLE) {
							cmd = GISUNLINK_CHARGE_LEISURE;
						}
						gisunlink_network_single_com_post(global,device_status.id,cmd);	
					} else if (device_status.id == 0xFF) { //获取全部端口状态
						//提交端口状态到服务端
						if(global->system.route_work == IS_ENABLE) {
							cmd = GISUNLINK_CHARGE_LEISURE;
						}
						gisunlink_network_multi_com_post(global,cmd);			
					}								
				}
				break;
			case GISUNLINK_EXIT_CHARGE_TASK://强制终止充电任务
				{							
					uint8_t canbestop = FALSE;
					uint8_t respond_value = FAILED;
					BYTES_TO_STRUCT(gisunlink_stop_charge_task,stop_charge,frame->cmd,frame->data,frame->data_len);						
					if(stop_charge.id < MAX_COM_QUANTITY) {
						if(stop_charge.force_stop) { //强制停止
							canbestop = TRUE;
						} else {
							if(global->comList[stop_charge.id].token == stop_charge.token) { //校验token一致后再停止
								canbestop = TRUE;
							}							
						}

						//停止充电
						if(canbestop) {
							gisunlink_lcd_set_port_status(stop_charge.id,PORT_LEISURE);
							if(global->comList[stop_charge.id].enable == IS_ENABLE) {		
								gisunlink_network_single_com_post(global,stop_charge.id,GISUNLINK_STOP_CHARGE);	
								gisunlink_com_reset(global,stop_charge.id,1);
								respond_value = gisunlink_com_stop_charge(global,stop_charge.id);
							}									
						}								
					} 
					
					gisunlink_network_module_respond(frame->id,frame->cmd,&respond_value,sizeof(uint8_t));

				}
				break;
			case GISUNLINK_SET_CONFIG://设备充电配置
				{
					uint8_t respond_value = 0;
					BYTES_TO_STRUCT(gisunlink_set_config,set_config,frame->cmd,frame->data,frame->data_len);						
					//设置全局的允许空负载通电几秒
					//全局->allow_zero_balance = set_config.allow_zero_balance;				
					global->system.no_load_time = set_config.allow_zero_balance;							
					gisunlink_network_module_respond(frame->id,frame->cmd,&respond_value,sizeof(uint8_t));											
				}
				break;
			case GISUNLINK_RESTART://设备重启
				{
					gisunlink_system_powerdown_backup();
					gisunlink_system_soft_reset();
				}
				break;
		}		
	}			
}

