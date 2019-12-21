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
	gisunlink_gpio_configuration();							//��ʼ��gpio
	gisunlink_key_init();												//��ʼ������
	gisunlink_network_module_init(USE_USART1);	//���ô���1��Ϊ��ģ��ͨ�Žӿ�
	gisunlink_debug_usart_init();								//����2��Ϊ���Դ���
	gisunlink_energy_metering_Init();						//��ʼ�����ܼ��	  	
	gisunlink_e2prom_Init();										//��ʼ��eeprom  
	gisunlink_lcd_init();												//��ʼ��LCD
	gisunlink_timer_init();											//��ʼ����ʱ��	
	//--------------init application--------------
	
	global = gisunlink_system_init();

	gisunlink_timer_register_post_data(gisunlink_system_tick_post_data);			//��ʱ�ϴ�״̬		
	gisunlink_timer_register_task_check(gisunlink_system_chargr_task_check);	//���������
	gisunlink_timer_register_normal_task(gisunlink_system_lcd_refresh);				//��Ļˢ��
	gisunlink_timer_register_capture_task(gisunlink_system_powerdown);				//ϵͳ����
	
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
