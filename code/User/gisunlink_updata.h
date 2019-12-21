#ifndef __GISUNLINK_UPDATA_H
#define __GISUNLINK_UPDATA_H

//include
#include "stm32f10x.h"
#include "stm32f10x_flash.h"
/****************************************************/
//C8---64K,CB---128K

//bootloader 8k
//code 		 28k
//code copy  28k
//parameter  0k

#define IAP_PARAMETER	0

#define CODE_COPY_START_ADDRESS			((uint32_t)0x08011000)	//��������ַ
#define CODE_COPY_END_ADDRESS			((uint32_t)0x08020000)	//��������ַ

//#define PARAMETER_START_ADDRESS		((uint32_t)0x0800e000)	//������ַ
// #define PARAMETER_END_ADDRESS		((uint32_t)0x08010000)	//������ַ

// #define BOOT_VERSION_ADDRESS		((uint32_t)0x0800e400)	//bootloader�汾��¼��ַ
// #define PRODUCTION_ADDRESS			((uint32_t)0x0800e800)	//��������
// #define SERIAL_ADDRESS				((uint32_t)0x0800eC00)	//���к�

//typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private define ------------------------------------------------------------*/
/* Define the STM32F10x FLASH Page Size depending on the used STM32 device */

uint8_t gisunlink_write_flash_by_update(uint32_t offset,uint8_t *pdata,uint16_t len);

void gisunlink_enable_updata(void);

#endif

