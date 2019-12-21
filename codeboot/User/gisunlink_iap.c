

#include "gisunlink_iap.h"
#include "gisunlink_eeprom.h"
#include "gisunlink_usart.h"
#include "stm32f10x_flash.h"

#define OK		0


// u32 __IAP_SIG = 0;
// static IAP_CMD cmd;
// #define SYSTEM_PARAMETER			0x700
#define IAP_PARAMETER				0//0(SYSTEM_PARAMETER + 112)   //

volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;




 union
{
	volatile uint32_t flag;
	volatile uint8_t fbuf[4];

}UpDataUn;

typedef  void (*pFunction)(void);




//系统软件复位
void sysSoftReset(void)
{
	__set_FAULTMASK(1);		// 关闭所有中端
	NVIC_SystemReset();		// 复位
}

// volatile uint32_t updataFlag=0;

//检查程序启动区域地址
void IAP_CheckBootLoader(void)
{
	//u8 cmd[4]={0};
	u16  iSize = 0;
	u32 wbuf,rbuf,i;
	vu32 JumpAddress;
	
	uint32_t EraseCounter = 0x00;
	uint32_t NbrOfPage = 0;
	
	
	pFunction Jump_To_Application;

// 	UpDataUn.flag=0x55aa55aa;
// 	ee_WriteBytes((unsigned char *)&UpDataUn.fbuf, IAP_PARAMETER, 4);
// 	UpDataUn.flag=0;
	ee_ReadBytes((unsigned char *)&UpDataUn.fbuf, IAP_PARAMETER, 4);

	if(UpDataUn.flag != 0x55aa55aa)
	{

		printf("\r\nEntering The Main Program Sectrion !! \r\n");

		JumpAddress = *(vu32*) (ASectionProgramAddress + 4);
		/* Jump to user application */
		Jump_To_Application = (pFunction) JumpAddress;
		/* Initialize user application's Stack Pointer */
 		__set_MSP(*(vu32*) ASectionProgramAddress); 
	
		Jump_To_Application();			
	}


	if(UpDataUn.flag == 0x55aa55aa)
	{
		printf("\r\nHave New Program Update !\r\n");

 		__set_PRIMASK(1);	//关总中断//CLI();	//关总中断
		FLASH_UnlockBank1();//FLASH_Unlock();

// 		printf("\r\nUnlock The Flash On MCU !\r\n");

// 		printf("\r\nErasing The Flash  Please Wait........\r\n");


		
		
	/* Define the number of page to be erased */
		NbrOfPage = (CODE_END_ADDRESS - CODE_START_ADDRESS) / FLASH_PAGE_SIZE;

		/* Clear All pending flags */
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	

		/* Erase the FLASH pages */
		for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
		{
			FLASHStatus = FLASH_ErasePage(CODE_START_ADDRESS + (FLASH_PAGE_SIZE * EraseCounter));
		}
		
// 		printf("\r\nFlash Erasing Finished !!!\r\n");
// 		printf("\r\nReading The New Program Date From  Flash .... \r\n");
		


		for(i=0;i<IAPProgramSize;i++) //  写B区64K程序进去A区
		{
			wbuf = (*(__IO uint32_t *)(BSectionProgramAddress + iSize ));//读出来
			
			FLASH_ProgramWord((ASectionProgramAddress + iSize ),wbuf);//写进去

			rbuf = (*(__IO uint32_t *)(ASectionProgramAddress + iSize ));//读出来
			
			if(wbuf != rbuf)
			{
				FLASH_LockBank1();
				__set_PRIMASK(0);	//开总中断//SEI();
				printf("\r\nUpdate Failed .......\r\n");
				sysSoftReset();	//软复位
				while(1);
			}		
		
			iSize +=4;
		}


		FLASH_LockBank1();
		__set_PRIMASK(0);	//开总中断//SEI();


		printf("\r\nUpdate is Successful !!\r\n");

// 		printf("\r\nSaving The Booting Flag And Reboot !!\r\n");


		
		for(i=0;i<10000;i++);

		UpDataUn.flag = 0;
		ee_WriteBytes((uint8_t *)UpDataUn.fbuf, IAP_PARAMETER, 4);
		//ee_ReadBytes(rbuf,IAP_PARAMETER,4);
		
		sysSoftReset();	//软复位	
		
	
	 }
	 else
	 {
	 	printf("\r\n No New Program Needed to be Update\r\n"); 
		 
	 }

	 	
}



