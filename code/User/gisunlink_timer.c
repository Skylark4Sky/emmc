/*
* _COPYRIGHT_
*
* File Name: gisunlink_timer.c
* System Environment: JOHAN-PC
* Created Time:2019-05-23
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#include <stdio.h>
#include <string.h>
#include "gisunlink.h"
#include "gisunlink_timer.h"
#include "gisunlink_gpio.h"
#include "Lcd_Driver.h"
#include "gisunlink_eeprom.h"
#include "gisunlink_gpio.h"

typedef struct {
	volatile uint8_t  timer_capture_sta ;//���벶��״̬		    				
	volatile uint16_t	timer_capture_val ;//���벶��ֵ
} CaptureTimerDataType;

typedef struct _gisunlink_timer_ctrl {
	GISUNLINK_TIMER_CALLBACK *task_check;
	GISUNLINK_TIMER_CALLBACK *post_data;	
	GISUNLINK_TIMER_CALLBACK *normal_task;	
	GISUNLINK_TIMER_CALLBACK *capture_task;	
	TIM_TypeDef *task_Timer;
	TIM_TypeDef *post_Timer;
	TIM_TypeDef *normal_Timer;
	TIM_TypeDef *capture_Timer;
} gisunlink_timer_ctrl;

static gisunlink_timer_ctrl timer_ctrl;
static CaptureTimerDataType CaptureTimerData;

static void gisunlink_timer_config(TIM_TypeDef *Timer,uint16_t Period,uint16_t Prescaler,uint8_t Priority) {
	uint32_t RCC_APB1Periph = 0;
	uint8_t IRQChannel = 0;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	TIM_DeInit(Timer);

	switch(*(uint32_t*)&Timer) {
		case TIM1_BASE:
			IRQChannel = TIM1_UP_IRQn;
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
			TIM_TimeBaseStructure.TIM_RepetitionCounter=0;
			break;
		case TIM2_BASE:
			IRQChannel = TIM2_IRQn;
			RCC_APB1Periph = RCC_APB1Periph_TIM2;
			break;
		case TIM3_BASE:
			IRQChannel = TIM3_IRQn;			
			RCC_APB1Periph = RCC_APB1Periph_TIM3;
			break;
		case TIM4_BASE:
			IRQChannel = TIM4_IRQn;
			RCC_APB1Periph = RCC_APB1Periph_TIM4;
			break;
	}

	RCC_APB1PeriphClockCmd(RCC_APB1Periph, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = Period - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = Prescaler - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(Timer, &TIM_TimeBaseStructure);

	TIM_ITConfig(Timer,TIM_IT_Update,ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ClearITPendingBit(Timer, TIM_IT_Update);
	TIM_Cmd(Timer, ENABLE);
}

static void gisunlink_capture_timer_config(TIM_TypeDef *Timer,uint16_t Period,uint16_t Prescaler,uint8_t Priority)  {
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_ICInitTypeDef  TIM2_ICInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//ʹ��TIM2ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);  //ʹ��GPIOBʱ��
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
	GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3;  //PB3 ���֮ǰ����  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_3);						

	//��ʼ����ʱ��2 TIM2	 
	TIM_TimeBaseStructure.TIM_Period = Period; //�趨�������Զ���װֵ 
	TIM_TimeBaseStructure.TIM_Prescaler = Prescaler; 	//Ԥ��Ƶ��   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	//��ʼ��TIM2���벶�����
	TIM2_ICInitStructure.TIM_Channel = TIM_Channel_2; //CC1S=01 	ѡ������� IC2ӳ�䵽TI1��
	TIM2_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
	TIM2_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
	TIM2_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
	TIM2_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
	TIM_ICInit(TIM2, &TIM2_ICInitStructure);

	//�жϷ����ʼ��
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = Priority;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� 

	TIM_Cmd(TIM2,DISABLE); 	//ʹ�ܶ�ʱ��2
	TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC2,ENABLE);//��������ж� ,����CC1IE�����ж�

}

void gisunlink_timer_init(void) {
	timer_ctrl.task_Timer = TIM1;
	timer_ctrl.capture_Timer = TIM2;	
	timer_ctrl.post_Timer = TIM3;
	timer_ctrl.normal_Timer = TIM4;	

	//72M
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	gisunlink_timer_config(timer_ctrl.task_Timer,9500,7200,2);	//950ms
	gisunlink_timer_config(timer_ctrl.post_Timer,10000,7200,3); //1s
	gisunlink_timer_config(timer_ctrl.normal_Timer,5000,7200,4); //500ms	
	gisunlink_capture_timer_config(timer_ctrl.capture_Timer,40000,72,0);
}

void gisunlink_timer_register_task_check(GISUNLINK_TIMER_CALLBACK *task_check) {

	if(task_check) {
		timer_ctrl.task_check = task_check;
	}
	return;

}

void gisunlink_timer_register_post_data(GISUNLINK_TIMER_CALLBACK *post_data) {
	if(post_data) {
		timer_ctrl.post_data = post_data;
	}
	return;
}

void gisunlink_timer_register_normal_task(GISUNLINK_TIMER_CALLBACK *normal_task) {
	if(normal_task) {
		timer_ctrl.normal_task = normal_task;
	}
	return;
}

void gisunlink_timer_register_capture_task(GISUNLINK_TIMER_CALLBACK *capture_task) {
	if(capture_task) {
		timer_ctrl.capture_task = capture_task;
	}
	return;
}

void gisunlink_timer_task_check_irq(void) {
	if(TIM_GetITStatus(timer_ctrl.task_Timer, TIM_IT_Update) != RESET) 
	{
		if(timer_ctrl.task_check) 
		{
			timer_ctrl.task_check();
		}		
		TIM_ClearITPendingBit(timer_ctrl.task_Timer, TIM_IT_Update);		
	} 	
}

void gisunlink_timer_post_data_irq(void) {
	if(TIM_GetITStatus(timer_ctrl.post_Timer, TIM_IT_Update) != RESET) 
	{
		if(timer_ctrl.post_data) 
		{
			timer_ctrl.post_data();
		}		
		TIM_ClearITPendingBit(timer_ctrl.post_Timer, TIM_IT_Update);		
	} 	
}

void gisunlink_timer_screen_refresh_irq(void) {
	if(TIM_GetITStatus(timer_ctrl.normal_Timer, TIM_IT_Update) != RESET) 
	{
		if(timer_ctrl.normal_task) 
		{
			timer_ctrl.normal_task();
		}		
		TIM_ClearITPendingBit(timer_ctrl.normal_Timer, TIM_IT_Update);		
	} 
}

void gisunlink_powerdown_ac_check_irq(void) {

	if(TIM_GetITStatus(timer_ctrl.capture_Timer, TIM_IT_Update) != RESET) {	  
		if(CaptureTimerData.timer_capture_sta&0XC0) {//��������
			if(timer_ctrl.capture_task) 
			{	
				timer_ctrl.capture_task();
			}
		}	 
	}

	if (TIM_GetITStatus(timer_ctrl.capture_Timer, TIM_IT_CC2) != RESET) //����2���������¼�
	{	
		if(CaptureTimerData.timer_capture_sta&0X40)		//����һ���½��� 		
		{	
			CaptureTimerData.timer_capture_sta=0;	
			CaptureTimerData.timer_capture_sta = 0x80;//CaptureTimerData.timer_capture_sta|=0X80;		//��ǳɹ�����һ��������
			CaptureTimerData.timer_capture_val=TIM_GetCapture2(TIM2);
			TIM_OC2PolarityConfig(TIM2,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
		}
		else  //��δ��ʼ,��һ�β���������
		{
			CaptureTimerData.timer_capture_sta=0;			//���
			CaptureTimerData.timer_capture_val=0;
			TIM_SetCounter(TIM2,0);
			CaptureTimerData.timer_capture_sta|=0X40;		//��ǲ�����������
			TIM_OC2PolarityConfig(TIM2,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
		}		    
	}			     	    					   

	TIM_ClearITPendingBit(timer_ctrl.capture_Timer, TIM_IT_CC2|TIM_IT_Update);
}

void gisunlink_set_timer_capture_sta(uint8_t sta) {
	CaptureTimerData.timer_capture_sta=sta;			//���
}

