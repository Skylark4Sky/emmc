#ifndef __DELAY_H
#define __DELAY_H 			   
#include "stm32f10x.h"
 
// void delay_init(void);
// void delay_ms(u16 nms);
// void delay_us(u32 nus);

#endif

void Delay(__IO uint32_t nCount);
void DelayUs(uint16_t nus);
void DelayMs(uint16_t nms);




























