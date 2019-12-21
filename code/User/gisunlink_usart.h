#ifndef __GISUNLINK_USART_H
#define __GISUNLINK_USART_H

//include
#include "stdio.h"	
#include "stm32f10x.h"
/****************************************************/
#define USART2_Enable()   // USART2->CR1 |= ((uint16_t)0x2000) 
#define USART2_Disable()  // USART2->CR1 &= ((uint16_t)0xDFFF);

typedef enum {
		UART1_INDEX,
		UART2_INDEX,
		UART3_INDEX
} UART_INDEX;


void  gisunlink_check_usart_init(void);
void gisunlink_debug_usart_init(void);

#endif


