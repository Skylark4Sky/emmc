/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "gisunlink_usart.h"
#include "gisunlink_system.h"
#include "gisunlink_gpio.h"
#include "gisunlink_lcd.h"
#include "gisunlink_key.h"
#include "gisunlink_timer.h"
#include "gisunlink_eeprom.h"
#include "gisunlink_network_module.h"
#include "gisunlink_energy_metering.h"

int main(void) {		
	gisunlink *global = NULL;
	
	//--------------config hardware---------------
	gisunlink_gpio_configuration();							//初始化gpio
	gisunlink_key_init();												//初始化按键
	gisunlink_network_module_init(USE_USART1);	//调用串口1作为和模块通信接口
	gisunlink_debug_usart_init();								//串口2作为调试串口
	gisunlink_energy_metering_Init();						//初始化电能检测	  	
	gisunlink_e2prom_Init();										//初始化eeprom  
	gisunlink_lcd_init();												//初始化LCD
	gisunlink_timer_init();											//初始化定时器	
	//--------------init application--------------
	
	global = gisunlink_system_init();

	gisunlink_timer_register_post_data(gisunlink_system_tick_post_data);			//定时上传状态		
	gisunlink_timer_register_task_check(gisunlink_system_chargr_task_check);	//充电任务检查
	gisunlink_timer_register_normal_task(gisunlink_system_lcd_refresh);				//屏幕刷新
	gisunlink_timer_register_capture_task(gisunlink_system_powerdown);				//系统掉电
	
	CaptureTimer_ClearITPendingBit();
	CaptureTimer_Enable();
	
	global->system.recvTime = gisunlink_system_get_time_tick();
	
	while (1) {
		gisunlink_network_module_read(gisunlink_system_network_message);
		gisunlink_system_check_route_work();
		gisunlink_read_realtime_meter_data(global);	
		gisunlink_key_scan(global);
	}	
}
