#ifndef __GISUNLINK_GPIO_H
#define __GISUNLINK_GPIO_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define LED						GPIO_Pin_12
#define LED_PORT              	GPIOA
#define LED_CLK               	RCC_APB2Periph_GPIOA
#define	LED_Off()				LED_PORT->BSRR = LED
#define	LED_On()				LED_PORT->BRR = LED
#define LED_Toggle()  			LED_PORT->ODR ^= LED


#define KEY1					GPIO_Pin_12
#define KEY1_PORT              	GPIOB
#define KEY1_CLK               	RCC_APB2Periph_GPIOB
#define KEY1_Status() 			GPIO_ReadInputDataBit(KEY1_PORT,KEY1)

#define KEY2					GPIO_Pin_13
#define KEY2_PORT              	GPIOB
#define KEY2_CLK               	RCC_APB2Periph_GPIOB
#define KEY12_Status() 			GPIO_ReadInputDataBit(KEY2_PORT,KEY2)

#define KEY3					GPIO_Pin_14
#define KEY3_PORT              	GPIOB
#define KEY3_CLK               	RCC_APB2Periph_GPIOB
#define KEY3_Status() 			GPIO_ReadInputDataBit(KEY3_PORT,KEY3)

#define KEY4					GPIO_Pin_15
#define KEY4_PORT              	GPIOB
#define KEY4_CLK               	RCC_APB2Periph_GPIOB
#define KEY4_Status() 			GPIO_ReadInputDataBit(KEY4_PORT,KEY4)

#define KEY5					GPIO_Pin_8
#define KEY5_PORT              	GPIOA
#define KEY5_CLK               	RCC_APB2Periph_GPIOA
#define KEY5_Status() 			GPIO_ReadInputDataBit(KEY5_PORT,KEY5)


#define CSCK					GPIO_Pin_15
#define CSCK_PORT              	GPIOA
#define CSCK_CLK               	RCC_APB2Periph_GPIOA
#define	CSCK_HIGH()				CSCK_PORT->BSRR = CSCK
#define	CSCK_LOW()				CSCK_PORT->BRR = CSCK

#define COE								GPIO_Pin_4
#define COE_PORT         	GPIOB
#define COE_CLK          	RCC_APB2Periph_GPIOB
#define	COE_HIGH()				COE_PORT->BSRR = COE
#define	COE_LOW()					COE_PORT->BRR = COE

#define CRCK							GPIO_Pin_5
#define CRCK_PORT        	GPIOB
#define CRCK_CLK          RCC_APB2Periph_GPIOB
#define	CRCK_HIGH()				CRCK_PORT->BSRR = CRCK
#define	CRCK_LOW()				CRCK_PORT->BRR = CRCK

#define CSER							GPIO_Pin_6
#define CSER_PORT         GPIOB
#define CSER_CLK          RCC_APB2Periph_GPIOB
#define	CSER_HIGH()				CSER_PORT->BSRR = CSER
#define	CSER_LOW()				CSER_PORT->BRR = CSER


#define ACCHK							GPIO_Pin_4
#define ACCHK_PORT        GPIOB
#define ACCHK_CLK         RCC_APB2Periph_GPIOB
#define	ACCHK_HIGH()			ACCHK_PORT->BSRR = ACCHK
#define	ACCHK_LOW()				ACCHK_PORT->BRR = ACCHK


#define I2C								GPIOB			
#define RCC_I2C_PORT 			RCC_APB2Periph_GPIOB	
#define I2C_SCL_PIN				GPIO_Pin_9			
#define I2C_SDA_PIN				GPIO_Pin_8			

/* 定义读写SCL和SDA的宏 */
#define I2C_SCL_1()  			I2C->BSRR = I2C_SCL_PIN				/* SCL = 1 */
#define I2C_SCL_0()  			I2C->BRR = I2C_SCL_PIN				/* SCL = 0 */

#define I2C_SDA_1()  			I2C->BSRR = I2C_SDA_PIN				/* SDA = 1 */
#define I2C_SDA_0() 			I2C->BRR = I2C_SDA_PIN				/* SDA = 0 */

#define I2C_SDA_READ()  		((I2C->IDR & I2C_SDA_PIN) != 0)	/* 读SDA口线状态 */
#define I2C_SCL_READ()  		((I2C->IDR & I2C_SCL_PIN) != 0)	/* 读SCL口线状态 */


/* 定义通信口选择模式 和模块电源开关 */
#define COMM							GPIO_Pin_7	
#define COMM_PORT					GPIOB		
#define COMM_CLK          RCC_APB2Periph_GPIOB
#define	COMM_HIGH()				COMM_PORT->BSRR = COMM
#define	COMM_LOW()				COMM_PORT->BRR = COMM

#define GSM								GPIO_Pin_8	
#define GSM_PORT					GPIOA	
#define GSM_CLK         	RCC_APB2Periph_GPIOA
#define	GSM_HIGH()				GSM_PORT->BSRR = GSM
#define	GSM_LOW()					GSM_PORT->BRR = GSM

#define POWERKEY					GPIO_Pin_11	
#define POWERKEY_PORT			GPIOA	
#define POWERKEY_CLK      RCC_APB2Periph_GPIOA
#define	POWERKEY_HIGH()		POWERKEY_PORT->BSRR = POWERKEY
#define	POWERKEY_LOW()		POWERKEY_PORT->BRR = POWERKEY

#define WIFI							GPIO_Pin_15	
#define WIFI_PORT					GPIOB		
#define WIFI_CLK          RCC_APB2Periph_GPIOB
#define	WIFI_HIGH()				WIFI_PORT->BSRR = WIFI
#define	WIFI_LOW()				WIFI_PORT->BRR = WIFI

enum {
	GISUNLINK_WIFI_MODULE,
	GISUNLINK_GSM_MODULE
};

void gisunlink_gpio_configuration(void);

void gisunlink_select_network_module(uint8_t module);

#endif


