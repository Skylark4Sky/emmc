#ifndef __GISUNLINK_APP_H
#define __GISUNLINK_APP_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define USART_DEBUG
#define EM_DEBUG

#define CHARGR_OVER_CURRENT 50			//���ػ��������
#define CHK_CURRENT_MA 			5

#define SUCCEED 		0	//�ɹ�
#define FAILED			1	//ʧ��

#define TURE 				1	//��
#define FALSE				0	//��

#define TIMER_10SECOND	10 //�����10���ύһ��
#define TIMER_60SECOND	60 //������60���ύһ��

#define IS_BREAKDOWN 	1	//����
#define NO_BREAKDOWN	0	//û����

#define IS_ENABLE 		1	//ʹ��
#define NO_ENABLE			0	//ûʹ��

#define IS_FALLING      1	//�½�
#define NO_FALLING      0

#define SINGLE_RECORD	1 	//������¼
#define MULTI_RECORD	MAX_COM_QUANTITY	//������¼

typedef enum {
	TIMER_UNKNOWN,
	TIMER_10S,
	TIMER_60S,
} TIMER_TYPE;

extern volatile uint8_t realTimeReadDateFlag;
extern volatile uint32_t energyOffset[10];

extern volatile uint8_t tim500msflag;
#endif


