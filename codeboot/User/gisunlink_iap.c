

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




//ϵͳ�����λ
void sysSoftReset(void)
{
	__set_FAULTMASK(1);		// �ر������ж�
	NVIC_SystemReset();		// ��λ
}

// volatile uint32_t updataFlag=0;

//���������������ַ
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

 		__set_PRIMASK(1);	//�����ж�//CLI();	//�����ж�
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
		


		for(i=0;i<IAPProgramSize;i++) //  дB��64K�����ȥA��
		{
			wbuf = (*(__IO uint32_t *)(BSectionProgramAddress + iSize ));//������
			
			FLASH_ProgramWord((ASectionProgramAddress + iSize ),wbuf);//д��ȥ

			rbuf = (*(__IO uint32_t *)(ASectionProgramAddress + iSize ));//������
			
			if(wbuf != rbuf)
			{
				FLASH_LockBank1();
				__set_PRIMASK(0);	//�����ж�//SEI();
				printf("\r\nUpdate Failed .......\r\n");
				sysSoftReset();	//��λ
				while(1);
			}		
		
			iSize +=4;
		}


		FLASH_LockBank1();
		__set_PRIMASK(0);	//�����ж�//SEI();


		printf("\r\nUpdate is Successful !!\r\n");

// 		printf("\r\nSaving The Booting Flag And Reboot !!\r\n");


		
		for(i=0;i<10000;i++);

		UpDataUn.flag = 0;
		ee_WriteBytes((uint8_t *)UpDataUn.fbuf, IAP_PARAMETER, 4);
		//ee_ReadBytes(rbuf,IAP_PARAMETER,4);
		
		sysSoftReset();	//��λ	
		
	
	 }
	 else
	 {
	 	printf("\r\n No New Program Needed to be Update\r\n"); 
		 
	 }

	 	
}



