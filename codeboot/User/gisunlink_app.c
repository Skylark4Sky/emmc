/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "gisunlink_usart.h"
#include "gisunlink_iap.h"
#include "gisunlink_eeprom.h"

//boot°æ±¾
//const unsigned char table[10] __at (BOOT_VERSION_ADDRESS) = {"SBL.01.000"};

typedef  void (*pFunction)(void);
void Delay(__IO uint32_t nCount);


/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	uint32_t JumpAddress;
	pFunction Jump_To_Application;

	gisunlink_uart_init(19200);

	printf("/*********************************************/\r\n");
	printf("/*      Bootloader  is  starting.......      */\r\n");
	printf("/*********************************************/\r\n");

 	gisunlink_e2prom_Init();
	IAP_CheckBootLoader();
	JumpAddress = *(vu32*) (ASectionProgramAddress + 4);
	Jump_To_Application = (pFunction) JumpAddress;
	__set_MSP(*(vu32*) ASectionProgramAddress); 	
	Jump_To_Application();	

	while (1){}
}

/**
  * @brief  Delay Function.
  * @param  nCount:specifies the Delay time length.
  * @retval None
  */
void Delay(__IO uint32_t nCount)
{
  while(nCount--)
  {
  }
}



