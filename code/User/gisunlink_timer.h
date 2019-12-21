/*
* _COPYRIGHT_
*
* File Name:gisunlink_timer.h
* System Environment: JOHAN-PC
* Created Time:2019-05-23
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#ifndef __GISUNLINK_TIMER_H__
#define __GISUNLINK_TIMER_H__

#include "stm32f10x.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define TaskTimer_Enable()					TIM1->CR1 |= TIM_CR1_CEN
#define TaskTimer_Disable()					TIM1->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN))

#define CaptureTimer_Enable()				TIM2->CR1 |= TIM_CR1_CEN
#define CaptureTimer_Disable()				TIM2->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN))

#define PostTimer_Enable()					TIM3->CR1 |= TIM_CR1_CEN
#define PostTimer_Disable()					TIM3->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN))

#define screenTimer_Enable()				TIM4->CR1 |= TIM_CR1_CEN
#define screenTimer_Disable()				TIM4->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN))

#define TaskTimer_ClearITPendingBit()		TIM1->SR = (uint16_t)~TIM_IT_Update
//#define CaptureTimer_ClearITPendingBit()	TIM2->SR = (uint16_t)~TIM_IT_Update
#define CaptureTimer_ClearITPendingBit()	 TIM2->SR = (uint16_t)~(TIM_IT_CC2|TIM_IT_Update)
#define PostTimer_ClearITPendingBit()		TIM3->SR = (uint16_t)~TIM_IT_Update
#define screenTimer_ClearITPendingBit()		TIM4->SR = (uint16_t)~TIM_IT_Update

typedef void GISUNLINK_TIMER_CALLBACK(void);	

void gisunlink_timer_init(void);

void gisunlink_timer_register_task_check(GISUNLINK_TIMER_CALLBACK *task_check);

void gisunlink_timer_register_post_data(GISUNLINK_TIMER_CALLBACK *post_data);

void gisunlink_timer_register_normal_task(GISUNLINK_TIMER_CALLBACK *normal_task);

void gisunlink_timer_register_capture_task(GISUNLINK_TIMER_CALLBACK *capture_task);

void gisunlink_timer_task_check_irq(void);

void gisunlink_timer_post_data_irq(void);

void gisunlink_timer_screen_refresh_irq(void);

void gisunlink_powerdown_ac_check_irq(void);

void gisunlink_set_timer_capture_sta(uint8_t sta);

#ifdef __cplusplus
}
#endif

#endif //__GISUNLINK_TIMER_H__
