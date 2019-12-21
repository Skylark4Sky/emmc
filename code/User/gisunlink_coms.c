/*
* _COPYRIGHT_
*
* File Name: gisunlink_coms.c
* System Environment: JOHAN-PC
* Created Time:2019-08-13
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#include <stdio.h>
#include <stdlib.h>
#include "gisunlink_lcd.h"
#include "gisunlink_coms.h"
#include "gisunlink_energy_metering.h"

typedef struct _gisunlink_com_charge_chk {
	volatile uint32_t current;											//当前充电电流(上一次)
	volatile uint32_t max_current;									//当前充电最大电流	
	volatile uint32_t min_current;									//当前充电最小电流
	volatile uint8_t count;													//空载统计
	volatile uint8_t is_falling;
	volatile uint16_t falling_count;
} gisunlink_com_charge_chk;

gisunlink_com_charge_chk comChkList[MAX_COM_QUANTITY] = {0};
uint8_t charge_over_time[MAX_COM_QUANTITY] = {0};

//复位端口电流
static void gisunlink_com_reset_current(charge_com *com) {	
	gisunlink_com_charge_chk *charge_chk = comChkList + com->id;
	charge_chk->current = 0;												//当前充电电流(上一次)	
	charge_chk->count = 0;													//空载检测统计	
	charge_chk->is_falling = NO_FALLING;						//是否持续下降
	charge_chk->falling_count = 0;									//下降次数统计
	charge_chk->max_current = 0;
	charge_chk->min_current = 0;		
}

static void gisunlink_com_reset_used(charge_com *com) {
	com->cur_current = 0;														//当前充电电流		
	com->utime = 0;																	//已冲电时间		
	com->used = 0;																	//已冲电度数		
}

//复位端口
void gisunlink_com_reset(gisunlink *global, uint8_t comID, uint8_t clearAll) {
	global->comList[comID].enable = NO_ENABLE;
	if(clearAll) {
		global->comList[comID].token = 0; 																//此次充电令牌
		global->comList[comID].total = 0;																	//此次最大充电量
		global->comList[comID].ptime = 0;																	//此次最大充电时间	
		global->comList[comID].max_current = 0;														//此次最大充电电流	
		global->comList[comID].cur_behavior = 0;		
		global->energyOffset[comID] = 0;		
		
		gisunlink_com_reset_used(&global->comList[comID]);		
	}
	global->icReset[comID] = 0;
	charge_over_time[comID] = 0;
	gisunlink_com_reset_current(&global->comList[comID]);							//复位充电电流相关
}

//置位端口
void gisunlink_com_set(gisunlink *global, uint8_t comID, gisunlink_charge_task *charge_task) {	
	global->comList[comID].token = charge_task->token; 								//此次充电令牌
	global->comList[comID].total = charge_task->total;								//此次最大充电量 						1度等于1000
	global->comList[comID].ptime = charge_task->ptime;								//此次最大充电时间	
	global->comList[comID].max_current = charge_task->max_current;		//此次最大充电电流
	global->comList[comID].cur_behavior = 0;	
	global->energyOffset[comID] = 0;		
	charge_over_time[comID] = 0;	
		
	gisunlink_com_reset_used(&global->comList[comID]);	
	gisunlink_com_reset_current(&global->comList[comID]);								//复位充电电流相关
}

//开始充电
uint8_t gisunlink_com_start_charge(gisunlink *global, uint8_t comID) {
	charge_over_time[comID] = 0;
	if(gisunlink_open_check_channel(comID) != SUCCEED) {
		global->comList[comID].breakdown |= OPEN_PORT_FAILED;						//打开通道失败
	} else {
		global->comList[comID].breakdown &= ~(OPEN_PORT_FAILED);
		return SUCCEED;
	}
	return FAILED;
}

//停止充电
uint8_t gisunlink_com_stop_charge(gisunlink *global, uint8_t comID) {
	gisunlink_close_check_channel(comID);	
	if(global->comList[comID].breakdown == NO_BREAKDOWN) {
		gisunlink_lcd_set_port_status(comID,PORT_LEISURE);					
	}
	return SUCCEED; 
}

static void gisunlink_com_chargring_status_post(gisunlink *global, uint8_t comID, void(*NetworkPost)(gisunlink *global, uint8_t comID, uint8_t behavior),uint8_t note_flags) {
	NetworkPost(global,comID,note_flags);
	gisunlink_com_reset(global,comID,0);
	gisunlink_com_stop_charge(global,comID);	
}

static void gisunlink_chargring_status_chk_init(gisunlink_com_charge_chk *chargr_chk, charge_com *com) {
	if(chargr_chk->max_current <= 0) {
		chargr_chk->max_current = com->cur_current;
	}

	if(chargr_chk->min_current <= 0) {
		chargr_chk->min_current = com->cur_current;
	}		

	if(com->cur_current > chargr_chk->max_current) {
		chargr_chk->max_current = com->cur_current;
	}

	if(com->cur_current < chargr_chk->min_current) {
		chargr_chk->min_current = com->cur_current;
	}		
}

//充电过程检测 检测是否充满以及是否出现空载
static void gisunlink_chargring_status_chk(gisunlink *global, uint8_t comID,void(*NetworkPost)(gisunlink *global, uint8_t comID, uint8_t behavior)) {

	gisunlink_com_charge_chk *chargr_chk = &comChkList[comID];
	gisunlink_chargring_status_chk_init(chargr_chk,&global->comList[comID]);

		if(global->comList[comID].cur_current <= CHARGR_OVER_CURRENT) { //电流小于 50ma 时，检测是否空载或者充满
			//如果连续no_load_time秒读到数据小于等于CHK_CURRENT_MA
			if(++chargr_chk->count >= global->system.no_load_time) {
				//如果最大电流和最小电流相等 一定未插充电设备
				if(chargr_chk->max_current == chargr_chk->min_current) {
					global->comList[comID].cur_behavior = GISUNLINK_CHARGE_NO_LOAD;
					gisunlink_com_chargring_status_post(global,comID,NetworkPost,GISUNLINK_CHARGE_NO_LOAD);	
					return;
				}

				//如果最小电流加上检测空载电流的值还小于最大电流。。那一定是瞬间拔掉	
				//CHARGR_OVER_CURRENT = 50 取30
				if((chargr_chk->min_current + 30) < chargr_chk->max_current) {
					global->comList[comID].cur_behavior = GISUNLINK_CHARGE_NO_LOAD;
					gisunlink_com_chargring_status_post(global,comID,NetworkPost,GISUNLINK_CHARGE_NO_LOAD);	
					return;					
				}

				if(abs((int)(chargr_chk->min_current - chargr_chk->current)) <= CHK_CURRENT_MA && chargr_chk->is_falling == IS_FALLING && chargr_chk->falling_count > 5) {
					global->comList[comID].cur_behavior = GISUNLINK_CHARGE_FINISH;
					gisunlink_com_chargring_status_post(global,comID,NetworkPost,GISUNLINK_CHARGE_FINISH);	//充满
					return;								
				} else {
					global->comList[comID].cur_behavior = GISUNLINK_CHARGE_NO_LOAD;
					gisunlink_com_chargring_status_post(global,comID,NetworkPost,GISUNLINK_CHARGE_NO_LOAD);	//空载
					return;							
				}

			}
		} else { //如果电流大于50ma时，检测是否电流是否存在连续跌落的状况			

			chargr_chk->count = 0;

			if(chargr_chk->current <= 0) {
				chargr_chk->current = global->comList[comID].cur_current;
			}

			//当前电流值不等于上次记录值时开始检测是否跌落
			if(global->comList[comID].cur_current != chargr_chk->current) {

				//当前采集到的电流值加上跌落差值还小于上一次记录的数值话 则存在电流跌落
				if((global->comList[comID].cur_current + CHK_CURRENT_MA) < chargr_chk->current) {			

					chargr_chk->is_falling = IS_FALLING;
					if(chargr_chk->falling_count < 0xFFFF) {
						chargr_chk->falling_count++;	//统计跌落次数							
					}					

					//记录当前跌落的电流值
					chargr_chk->current = global->comList[comID].cur_current;	

				}
			}
		}
}

void gisunlink_chargring_chk(gisunlink *global,void(*NetworkPost)(gisunlink *global, uint8_t comID, uint8_t behavior)) {
	uint8_t comID = 0;

	for(comID = 0; comID < MAX_COM_QUANTITY; comID++) {
		if(global->comList[comID].enable == IS_ENABLE) {	
			//global->comList[comID].utime++;	
			if(global->comList[comID].breakdown == NO_BREAKDOWN) { 
				//如果充电时间已到
				if(global->comList[comID].utime >= global->comList[comID].ptime) {
					//充电已完成
					gisunlink_lcd_set_port_status(comID,PORT_LEISURE);					
					global->comList[comID].cur_behavior = GISUNLINK_CHARGE_FINISH;
					gisunlink_com_chargring_status_post(global,comID,NetworkPost,GISUNLINK_CHARGE_FINISH);
					continue;
				}
				
				//充电的度数已到
				if((global->comList[comID].used + global->energyOffset[comID]) >= global->comList[comID].total) {
					uint8_t chk_time = charge_over_time[comID];
					if(chk_time++ >= 30) {
						//充电已完成
						global->comList[comID].cur_behavior = GISUNLINK_CHARGE_FINISH;						
						gisunlink_lcd_set_port_status(comID,PORT_LEISURE);
						gisunlink_com_chargring_status_post(global,comID,NetworkPost,GISUNLINK_CHARGE_FINISH);
						//复位检测次数统计
						charge_over_time[comID] = 0;
						continue;
					}
					charge_over_time[comID] = chk_time;
				} else {
					//复位检测次数统计
					charge_over_time[comID] = 0;
				}
	
				//未充满
				gisunlink_lcd_set_port_status(comID,PORT_CHARGEING);
				//充电状态检测
				gisunlink_chargring_status_chk(global,comID,NetworkPost);	
			} else {
				gisunlink_lcd_set_port_status(comID,PORT_BREAKDOWN);
				//报故障状态到服务器	
				global->comList[comID].cur_behavior = GISUNLINK_CHARGE_BREAKDOWN;		
				gisunlink_com_chargring_status_post(global,comID,NetworkPost,GISUNLINK_CHARGE_BREAKDOWN);
			}
		} else { //如果是没有开启的情况下检测是否异常 并设置 相应 屏幕 标志
			if(global->comList[comID].breakdown != NO_BREAKDOWN && global->breakdown_chk[comID] == NO_BREAKDOWN) {		
				gisunlink_lcd_set_port_status(comID,PORT_BREAKDOWN);//故障
					if(global->comList[comID].breakdown_time >= 10) {
						global->breakdown_chk[comID] = IS_BREAKDOWN;
						global->comList[comID].breakdown_time = 0;
						if(gisunlink_com_start_charge(global,comID) == SUCCEED) { //尝试打开端口
							global->comList[comID].breakdown = NO_BREAKDOWN;
							global->breakdown_chk[comID] = NO_BREAKDOWN;
							gisunlink_com_reset(global,comID,0);
							gisunlink_lcd_set_port_status(comID,PORT_LEISURE);//空闲												
						}			
						gisunlink_com_stop_charge(global,comID);							
					}						
			}
		}
	}
}
