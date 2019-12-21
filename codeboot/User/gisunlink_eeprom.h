#ifndef __GISUNLINK_E2PROM_H__
#define __GISUNLINK_E2PROM_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"


#define EE_MODEL_NAME		"AT24C64"
#define EE_DEV_ADDR			0xA0		/* �豸��ַ */
#define EE_PAGE_SIZE		32			/* ҳ���С(�ֽ�) */
#define EE_SIZE				256			/* ������(�ֽ�) */
#define EE_ADDR_BYTES		2			/* ��ַ�ֽڸ��� */
#define I2C_WR	0		/* д����bit */
#define I2C_RD	1		/* ������bit */




void i2c_Stop(void);
void gisunlink_e2prom_Init(void);
uint8_t ee_WriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize);
uint8_t ee_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize);




#endif  /*__E2PROM_H__*/
