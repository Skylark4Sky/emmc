

#ifndef _GISUNLINK_IAP_H_
#define _GISUNLINK_IAP_H_

#ifndef __at
#define __at(_addr) __attribute__ ((at(_addr)))

#endif


/*


	Flash 区域结构

	 ----------------- 	<------0x08000000
	|		IAP		  |
	|	  (8KB)	  |
	|-----------------|	<------0x08002000
	|				  |
	|				  |
	|	用户程序A区	  |
	|	  (60KB)	  |
	|				  |
	|				  |	
	|-----------------|	<------0x08011000
	|				  |
	|				  |
	|	用户程序B区	  |
	|	  (60KB)	  |
	|				  |
	|				  |	
	 -----------------	<------0x08020000	

*/


#define FlashProgramSize	0xF000 //60KB size
#define IAPProgramSize		0x3C00 // 60K //0x1800//=0x6000/4		//24KB size

#define IAPAddressAddress			0x08000000		//引导程序的起始地址
#define	ASectionProgramAddress		0x08002000		//A区程序起始地址
#define	BSectionProgramAddress		0x08011000		//B区程序起始地址

#define CODE_START_ADDRESS			((uint32_t)0x08002000)	//代码区地址
#define CODE_END_ADDRESS			((uint32_t)0x08011000)	//代码区地址

#define CODE_COPY_START_ADDRESS			((uint32_t)0x08011000)	//备份区地址
#define CODE_COPY_END_ADDRESS			((uint32_t)0x08020000)	//备份区地址


#define FLASH_PAGE_SIZE    ((uint16_t)0x400)



#define IAP_SINGLE					0xA55A5AA5

#define IAP_ASECTOR					0xA00A0AA0
#define IAP_BSECTOR					0xB00B0BB0

// cmd
#define IAP_UPDATE_RUN_BL			0xAA

#define IAP_UPDATE_APP_DEF			0xA0
#define IAP_UPDATE_APP_ADD			0xA1
#define IAP_UPDATE_BL				0xA2


#define IAP_MAXUSERFLASH_SIZE		580		//240KB / 0.5(KB per Packet)		


extern void IAP_CheckBootLoader(void);


// typedef __packed struct _IAP_CMD
// {
// 	unsigned char flag;
// 	unsigned long sig;
// 	unsigned char cmd;
// 	unsigned long addr;
// 	unsigned long psize;
// 	unsigned char unuse[3];
// }IAP_CMD;




#endif




