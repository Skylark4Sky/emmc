#ifndef __GISUNLINK_EEPROM_H__
#define __GISUNLINK_EEPROM_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define SYSTEM_STATE_ADDRESS	256
#define DEVICE_COM_ADDRESS	512

#define EE_MODEL_NAME		"AT24C64"
#define EE_DEV_ADDR			0xA0		/* 设备地址 */
#define EE_PAGE_SIZE		32			/* 页面大小(字节) */
#define EE_SIZE				256			/* 总容量(字节) */
#define EE_ADDR_BYTES		2			/* 地址字节个数 */
#define I2C_WR	0		/* 写控制bit */
#define I2C_RD	1		/* 读控制bit */

void i2c_Stop(void);
void gisunlink_e2prom_Init(void);
uint8_t ee_WriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize);
uint8_t ee_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize);

#endif  /*__E2PROM_H__*/
