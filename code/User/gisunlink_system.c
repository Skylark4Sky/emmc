/*
* _COPYRIGHT_
*
* File Name: gisunlink_system.c
* System Environment: JOHAN-PC
* Created Time:2019-08-13
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Lcd_Driver.h"
#include "gisunlink_lcd.h"
#include "gisunlink_key.h"
#include "gisunlink_conf.h"
#include "gisunlink_type.h"
#include "gisunlink_coms.h"
#include "gisunlink_gpio.h"
#include "gisunlink_timer.h"
#include "gisunlink_system.h" 
#include "gisunlink_network_task.h"
#include "gisunlink_network_module.h"

gisunlink global = {0};

static void gisunlink_system_read_stmID(uint8_t *deviceSN) {
	vu8* addr = (vu8*)(0x1ffff7e8);
	uint8_t i = 0;
	for(i = 0; i < STM32_UNIQUEID_SIZE; ++i) {
		uint8_t id= *addr;
		deviceSN[i] = id;
		++addr;
	}
}

//初始化端口
static void gisunlink_system_init_coms(gisunlink *global) {
	uint8_t comID = 0;

	for(comID = 0; comID < MAX_COM_QUANTITY; comID++) {
		global->comList[comID].id = comID;
		global->comList[comID].breakdown = NO_BREAKDOWN;
		global->breakdown_chk[comID] = NO_BREAKDOWN;
		gisunlink_com_reset(global,comID,1);
	}
}

static void gisunlink_system_check_com_state(gisunlink *global) {
	uint8_t comID = 0;
	PORT_STATUS lcdstate = PORT_LEISURE; 
	//读本地存储数据
	for(comID = 0; comID < MAX_COM_QUANTITY; comID++) {
			if(global->comList[comID].breakdown == NO_BREAKDOWN && global->comList[comID].enable == IS_ENABLE) { //如果正在充电
				if(gisunlink_com_start_charge(global,comID) == FAILED) {//启用充电通道
					if(global->comList[comID].breakdown != NO_BREAKDOWN) {
						lcdstate = PORT_BREAKDOWN;
					}
				} else {
					lcdstate = PORT_CHARGEING;
				}
			} else if(global->comList[comID].enable == NO_ENABLE) {  //如果没有正在充电
				gisunlink_com_stop_charge(global,comID);//关闭充电通道
				lcdstate = PORT_LEISURE;
			} 
			gisunlink_lcd_set_port_status(comID,lcdstate);
		}
}

uint8_t chkComID(gisunlink *global) {
	uint8_t retBool = 0, comID = 0;
	for(comID = 0; comID < MAX_COM_QUANTITY; comID++) {
		if(global->comList[comID].id == 0xFF) {
			global->comList[comID].id = comID;
			gisunlink_com_reset(global,comID,1);
			retBool = 1;
		}
	}
	return retBool;
}

gisunlink *gisunlink_system_init(void) {

	global.system_update_flag = FALSE;
	
	global.system.route_work = NO_ENABLE;
	global.system.network_connect = FALSE;
	global.system.no_load_time = NO_LOAD_TIME;
	global.system.network_state = GISUNLINK_NETMANAGER_IDLE; 

	gisunlink_system_read_stmID((uint8_t *)global.system.deviceSN);
	gisunlink_system_init_coms(&global);

	//是否为第一次运行
	if(gisunlink_conf_init(&global) != SYSTEM_INIT) {
		gisunlink_set_comList_conf(&global);
	} else {
		gisunlink_get_comList_conf(&global);//读s取上次断电前保存的数据	
		gisunlink_system_check_com_state(&global);//获取通道状态、更新LCD内容
	}

	if(chkComID(&global)) {
		gisunlink_set_comList_conf(&global);		
	}
	
	char version[16] = {0};
	snprintf(version,16,"%s%s","GSL",FIRMWARE_VERSION);
	
	gisunlink_lcd_set_bottom(version,VERSION_STRING);	

	if(global.conf.mode == SYSTEM_GSM_MODE) {
		gisunlink_select_network_module(GISUNLINK_GSM_MODULE);		
	} else {
		gisunlink_select_network_module(GISUNLINK_WIFI_MODULE);
	}
	return &global;
}

void gisunlink_system_network_message(gisunlink_recv_frame *frame) {
	switch(frame->cmd) {
		case GISUNLINK_NETWORK_STATUS:
			if(frame->dir == GISUNLINK_COMM_REQ) { //先回复
				gisunlink_network_module_respond(frame->id,frame->cmd,NULL,0);
			}
			if(frame->data_len) {
				//网络状态
				GISUNLINK_NETMANAGER_WORK_STATE state = (GISUNLINK_NETMANAGER_WORK_STATE)*frame->data;				
				if(state != global.system.network_state) {
					global.system.network_state = state;					
					switch(global.system.network_state) {
						case GISUNLINK_NETMANAGER_IDLE:
							global.system.get_deviceSn = FALSE;
							break;												
						case GISUNLINK_NETMANAGER_TIME_SUCCEED:
							global.system.state_recvTime = gisunlink_system_get_time_tick();							
							break;
						case GISUNLINK_NETMANAGER_CONNECTED_SER:
							global.system.network_connect = TURE;
							break;
						case GISUNLINK_NETMANAGER_DISCONNECTED_SER:
							global.system.recvTime = gisunlink_system_get_time_tick();
							global.system.network_connect = FALSE;
							break;
					}
					gisunlink_lcd_refresh_network_state(global.system.network_state);
				}
			}						
			break;
		case GISUNLINK_NETWORK_RSSI:
			if(frame->dir == GISUNLINK_COMM_REQ) { //先回复
				gisunlink_network_module_respond(frame->id,frame->cmd,NULL,0);
				if(frame->data_len) {
					//网络信号
					signed char apRssi = *frame->data;
					global.system.signal = apRssi;
					gisunlink_lcd_set_apRssi(apRssi);
				}
			}			
			break;			
		case GISUNLINK_DEV_FW_INFO:
			gisunlink_dev_fw_info(&global,frame);
			break;			
		case GISUNLINK_DEV_FW_TRANS:
			gisunlink_dev_fw_trans(&global,frame);
			break;		
		case GISUNLINK_DEV_FW_READY:
			gisunlink_dev_fw_ready(&global,frame);
			break;	
		case GISUNLINK_DEV_SN:
			if(frame->dir == GISUNLINK_COMM_REQ) { //带值回复
				gisunlink_network_module_respond(frame->id,frame->cmd,NULL,0);									
				if(frame->data_len) {
					if(global.system.get_deviceSn == FALSE) {
						global.system.get_deviceSn = TRUE;
						gisunlink_lcd_set_bottom((char *)frame->data,SN_STRING);
					}
				}					
			}						
			break;					
		case GISUNLINK_HW_SN:
			if(frame->dir == GISUNLINK_COMM_REQ) { //带值回复				
				gisunlink_network_module_respond(frame->id,frame->cmd,global.system.deviceSN,STM32_UNIQUEID_SIZE);
			}				
			break;
		case GISUNLINK_FIRMWARE_VERSION:
			if(frame->dir == GISUNLINK_COMM_REQ) { //带值回复				
				gisunlink_network_module_respond(frame->id,frame->cmd,(uint8_t *)FIRMWARE_VERSION,12);
			}					
			break;
		case GISUNLINK_TASK_CONTROL:
			gisunlink_task_control(&global,frame);
			break;
		default:
			break;
	}
}

uint8_t gisunlink_system_check_route_work(void) {
	uint8_t comID = 0;

	for(comID = 0; comID < MAX_COM_QUANTITY; comID++) {
		if(global.comList[comID].enable == IS_ENABLE) {
			global.system.route_work = IS_ENABLE;
			return IS_ENABLE; 
		}
	}
	
	global.system.route_work = NO_ENABLE;
	return global.system.route_work;
}

//定时发送设备端口状态到服务器
void gisunlink_system_tick_post_data(void) {
	uint8_t second = TIMER_60SECOND;
	uint8_t cmd = GISUNLINK_CHARGE_LEISURE;

	global.system.post_tick++;

	//获取无线模组SN号	
	if(global.system.get_deviceSn == FALSE) {
		gisunlink_network_module_write(GISUNLINK_DEV_SN,NULL,0);	
	}	

	if(global.system.route_work == IS_ENABLE) {
		cmd = GISUNLINK_CHARGEING;
		second = TIMER_10SECOND;
	} 	

	if(global.system.post_tick >= second) {
		//提交端口状态到服务端
		gisunlink_network_multi_com_post(&global,cmd);
		global.system.post_tick = 0;
	}	

	gisunlink_network_module_write(GISUNLINK_NETWORK_STATUS,NULL,0);
}

void gisunlink_system_lcd_refresh(void) {
	//LCD刷新
	if(global.system_update_flag == FALSE) {
		#if 1	//时间通步成功后检查还原上个状态
		if(global.system.network_state == GISUNLINK_NETMANAGER_TIME_SUCCEED && global.system.network_connect == TRUE) {
			if((gisunlink_system_get_time_tick() - global.system.state_recvTime) > 5000) {
					global.system.network_state = GISUNLINK_NETMANAGER_CONNECTED_SER;	
					gisunlink_lcd_refresh_network_state(global.system.network_state);
			}
		}
		#endif
		gisunlink_lcd_refresh();
	} else {
		global.system.firmware.transfer_timeout++;
		//升级中
		
		if(global.system.firmware.transfer_timeout == 30) {
			gisunlink_lcd_set_status(LCD_STATE_UPDATE_TIMEOUT);
			gisunlink_lcd_refresh();			
		}

		if(global.system.firmware.transfer_timeout == 36) {
			gisunlink_lcd_set_status(LCD_STATE_EXIT_UPDATE);
			gisunlink_lcd_refresh();
		}

		if(global.system.firmware.transfer_timeout >= 40) {
			global.system.firmware.transfer_timeout = 0;
			global.system_update_flag = FALSE;
			gisunlink_lcd_refresh_network_state(global.system.network_state);		
		}
	}
}

void gisunlink_system_chargr_task_check(void) {
	gisunlink_chargring_chk(&global,gisunlink_network_single_com_post); 
}

void gisunlink_system_time_tick(void) {
	uint8_t comID = 0;
	global.system.tick++;	
	
	gisunlink_key_system_scan();	
	
	if(global.system.tick%1000 == 0) {		
		
		if(global.system.network_connect == FALSE) {
			uint32_t offLineTime = (global.system.tick - global.system.recvTime);			
			
			if(offLineTime >= 900000) {//15分钟	
			#if 0	
				//如果离线15分钟 重启板子
				if(global.system.route_work == IS_ENABLE) { 
					gisunlink_system_powerdown_backup();
				}
				gisunlink_system_soft_reset();	
			#else 
				//如果离线15分钟 设备没有在充电 则 重启整个板子 如果在充电则重启2G模组
				global.system.recvTime = global.system.tick;
				if(global.system.route_work == IS_ENABLE) { 
					GSM_HIGH();
					WIFI_HIGH();							
					if(global.conf.mode == SYSTEM_GSM_MODE) {
						gisunlink_select_network_module(GISUNLINK_GSM_MODULE);		
					} else {
						gisunlink_select_network_module(GISUNLINK_WIFI_MODULE);
					}				
				} else {
					gisunlink_system_soft_reset();
				}
			#endif
			}			
		}

		for(comID = 0; comID < MAX_COM_QUANTITY; comID++)	{
			if(global.comList[comID].enable == IS_ENABLE) {	
				global.comList[comID].utime++;	
			}					
			if(global.comList[comID].breakdown != NO_BREAKDOWN && global.breakdown_chk[comID] == NO_BREAKDOWN) {	
				global.comList[comID].breakdown_time++;
			}	
		}
	}
}

uint32_t gisunlink_system_get_time_tick(void) {
	return global.system.tick;
}

void gisunlink_system_powerdown_backup(void) {
	//无交流信号处理
	GSM_HIGH();
	WIFI_HIGH();
	LCD_LED_CLR;// 关闭液晶背光

	TaskTimer_Disable();
	PostTimer_Disable();
	screenTimer_Disable();			
	CaptureTimer_Disable();//关闭定时器
	__set_PRIMASK(1);//关闭中断
	gisunlink_set_comList_conf(&global);
	gisunlink_set_timer_capture_sta(0);
	CaptureTimer_Enable();	
	__set_PRIMASK(0);//开中断

	LCD_LED_SET;// 打开液晶背光				

	if(global.conf.mode == SYSTEM_GSM_MODE) {
		gisunlink_select_network_module(GISUNLINK_GSM_MODULE);		
	} else {
		gisunlink_select_network_module(GISUNLINK_WIFI_MODULE);
	}

	TaskTimer_Enable();
	PostTimer_Enable();
	screenTimer_Enable();			
	CaptureTimer_Enable();	
}

void gisunlink_system_powerdown(void) {
	if(global.system_update_flag == FALSE) {		//global.system.route_work		
		gisunlink_system_powerdown_backup();
	}
}

//系统软件复位
void gisunlink_system_soft_reset(void) {
	__set_FAULTMASK(1);		// 关闭所有中端
	NVIC_SystemReset();		// 复位
}
