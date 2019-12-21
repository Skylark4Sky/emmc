
#include "gisunlink_delay.h"


void Delay(__IO uint32_t nCount)
{ 
	for(; nCount != 0; nCount--);

}

void DelayUs(uint16_t nus)//2-2.4us;3-3.2us;30-26us
{
	uint32_t temp;
	//while(nus--)
	for(;nus>0;nus--)
		for(temp=5;temp>0; temp--);//9

}

void DelayMs(uint16_t nms)//1-1.2ms;10-12ms;for25-30ms
{
	uint32_t temp;
	while(nms--)
		for(temp=9900;temp>0; temp--);//11900

}



























