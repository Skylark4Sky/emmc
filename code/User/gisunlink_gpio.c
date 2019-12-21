
/* Includes ------------------------------------------------------------------*/ 

#include "gisunlink_gpio.h"
#define GPIO_Remap_SWJ_JTAGDisable  ((uint32_t)0x00300200)  /*!< JTAG-DP Disabled and SW-DP Enabled */

void gisunlink_gpio_configuration(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE); 
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
	    
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12  ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ; 
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
//--595	
	GPIO_InitStructure.GPIO_Pin =  COE;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 
	GPIO_Init(COE_PORT, &GPIO_InitStructure);
	COE_HIGH();
	
	GPIO_InitStructure.GPIO_Pin =  CSCK;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 
	GPIO_Init(CSCK_PORT, &GPIO_InitStructure);
	CSCK_LOW();
	
	GPIO_InitStructure.GPIO_Pin =  CRCK;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 
	GPIO_Init(CRCK_PORT, &GPIO_InitStructure);
	CRCK_LOW();
	
	GPIO_InitStructure.GPIO_Pin =  CSER;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 
	GPIO_Init(CSER_PORT, &GPIO_InitStructure);
	CSER_LOW();
}

void gisunlink_select_network_module(uint8_t module) {
	GPIO_InitTypeDef GPIO_InitStructure;	
	
	GPIO_InitStructure.GPIO_Pin = COMM;		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	
	GPIO_Init(COMM_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GSM;	
	GPIO_Init(GSM_PORT, &GPIO_InitStructure);	
	GPIO_InitStructure.GPIO_Pin = WIFI;		
	GPIO_Init(WIFI_PORT, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = POWERKEY;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 
	GPIO_Init(POWERKEY_PORT, &GPIO_InitStructure);	

	if(module == GISUNLINK_GSM_MODULE) {
		COMM_HIGH();		
		WIFI_HIGH();
		POWERKEY_LOW();		
		GSM_LOW();	
	} else {
		COMM_LOW();
		GSM_HIGH();
		POWERKEY_HIGH();
		WIFI_LOW();			
	}
}
