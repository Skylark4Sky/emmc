
#include "gisunlink_updata.h"	
#include "gisunlink_usart.h"	
#include "gisunlink_eeprom.h"
#include "gisunlink_delay.h"
#include "gisunlink_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHIP_FLASH_SIZE 128	 			//(单位为K)
//////////////////////////////////////////////////////////////////////////////////////////////////////

#define FLASH_RW_SIZE 4

#if CHIP_FLASH_SIZE < 256
#define SECTOR_SIZE 1024 //字节
#else
#define SECTOR_SIZE	2048
#endif		

#define WRITE_DATA_SIZE 256

#define FLASH_BOOT_BASE 0x08000000 	//STM32 FLASH的起始地址

uint32_t FLASH_PAGE_BUF[SECTOR_SIZE/FLASH_RW_SIZE];
uint32_t CMP_DATA_BUF[FLASH_RW_SIZE];
uint32_t WRITE_DATA_BUF[WRITE_DATA_SIZE/FLASH_RW_SIZE];

/*********************************************/
void gisunlink_enable_updata(void) {
	uint32_t wbuf= 0x55aa55aa;
	ee_WriteBytes((uint8_t *)&wbuf, IAP_PARAMETER, 4);//写升级使能	
	/*回复正在升级*/
	Delay(6000000);
	gisunlink_system_soft_reset();	
	while(1);
}

static void gisunlink_flash_read_data(uint32_t address,uint32_t *buf, uint16_t len) {
	uint16_t i;
	for(i = 0; i < len; i++) {
		buf[i] = (*(__IO uint32_t *)address);
    address += FLASH_RW_SIZE;        
  }
}

static uint8_t gisunlink_flash_write_data(uint32_t address, uint32_t *buf, uint16_t len) {
	uint16_t i; uint8_t ret = 0;
	uint32_t *data = NULL;
	for(i = 0; i < len; i++) {
		data = &buf[i];
		if(FLASH_ProgramWord(address,buf[i]) == FLASH_COMPLETE) {
			gisunlink_flash_read_data(address,CMP_DATA_BUF,1);
			if(memcmp(data,CMP_DATA_BUF,FLASH_RW_SIZE) == 0) {
				 address += FLASH_RW_SIZE; 
			} else {
				ret = 1;
				break;				
			}
		} else {
			ret = 1;
			break;
		} 
  }
	return ret;
}

uint8_t gisunlink_write_flash_by_update(uint32_t offset,uint8_t *pdata,uint16_t len) {	
 	uint16_t i; uint8_t ret = 0; //ret 0 成功
	uint32_t WriteAddr = CODE_COPY_START_ADDRESS + offset;
	uint32_t flash_offset = WriteAddr - FLASH_BOOT_BASE;   		
	uint16_t write_offset = len / FLASH_RW_SIZE;
	uint32_t page_pos = flash_offset / SECTOR_SIZE;	  	 										//扇区地址
	uint16_t page_offset = (flash_offset%SECTOR_SIZE)/FLASH_RW_SIZE;	   		//扇区内偏移地址
	uint16_t page_remain_buf = SECTOR_SIZE/FLASH_RW_SIZE - page_offset; 			//扇区内剩余地址
	uint32_t page_address = 0;
	uint8_t data[FLASH_RW_SIZE];
		
	if(len == 0) {
		return 1;
	}
	
	if(WriteAddr < FLASH_BOOT_BASE||(WriteAddr >= (FLASH_BOOT_BASE + 1024*CHIP_FLASH_SIZE))) {
		return 1;
	}
	
	if(write_offset <= page_remain_buf) {
		page_remain_buf = write_offset;	
	}
	
	__set_PRIMASK(1);	
	/* Unlock the Flash to enable the flash control register access *************/ 
	FLASH_UnlockBank1();
	/* Clear All pending flags */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);		

	while(1) {	
		page_address = (page_pos * SECTOR_SIZE) + FLASH_BOOT_BASE;
		gisunlink_flash_read_data(page_address,FLASH_PAGE_BUF,SECTOR_SIZE/FLASH_RW_SIZE);//读出整个扇区的内容
		for(i = 0; i < page_remain_buf; i++) {
			if(FLASH_PAGE_BUF[page_offset + i] != 0xFFFFFFFF) break;  	  
		}
		
		if(i < page_remain_buf) {
			if(FLASH_ErasePage(page_address) != FLASH_COMPLETE) {
				ret = 1;
				break;
			}
			for(i = 0; i < page_remain_buf; i++) {
				data[0] = *pdata++;
				data[1] = *pdata++;
				data[2] = *pdata++;
				data[3] = *pdata++;
				memcpy(&FLASH_PAGE_BUF[i + page_offset],data,FLASH_RW_SIZE);
			}
			if(gisunlink_flash_write_data(page_address,FLASH_PAGE_BUF,SECTOR_SIZE/FLASH_RW_SIZE) != 0) {
				ret = 1;
				break;				
			}
		} else {			
			for(i = 0; i < page_remain_buf; i++) {
				data[0] = *pdata++;
				data[1] = *pdata++;
				data[2] = *pdata++;
				data[3] = *pdata++;
				memcpy(&WRITE_DATA_BUF[i],data,FLASH_RW_SIZE);				
			}			
			if(gisunlink_flash_write_data(WriteAddr,WRITE_DATA_BUF,page_remain_buf) != 0) {
				ret = 1;
				break;				
			}
		}
		
		if(write_offset == page_remain_buf) {
			break;																								//写入结束了
		} else {
			page_pos++;																						//扇区地址增1
			page_offset = 0;																			//偏移位置为0 	 
		  pdata += (page_remain_buf * FLASH_RW_SIZE); 					//指针偏移
			WriteAddr += page_remain_buf;													//写地址偏移	   
		  write_offset -= page_remain_buf;	
			if(write_offset > (SECTOR_SIZE/FLASH_RW_SIZE)) {
				page_remain_buf = SECTOR_SIZE / FLASH_RW_SIZE;			//下一个扇区还是写不完
			}	else {
				page_remain_buf = write_offset;											//下一个扇区可以写完了
			}
		}
	};

	FLASH_LockBank1();
	__set_PRIMASK(0);	
	//回复写flash完成	
	return ret;
}
