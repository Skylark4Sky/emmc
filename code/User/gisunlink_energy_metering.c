/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "gisunlink_energy_metering.h"
#include "gisunlink_type.h"
#include "gisunlink_usart.h"
#include "gisunlink_gpio.h"
#include "gisunlink_delay.h"
//----------------------start RN8209  ��ַ����---------------------------------------------------//
#define					ADSYSCON 				0x00 
#define        	ADEMUCON 				0x01
#define        	ADHFConst     	0x02 
#define        	ADPStart      	0x03 
#define        	ADDStart      	0x04 
#define					ADGPQA        	0x05 
#define        	ADGPQB        	0x06 
#define        	ADPhsA        	0x07 
#define        	ADPhsB        	0x08
#define					ADQPHSCAL				0x09    
#define					ADAPOSA 				0x0a
#define        	ADAPOSB 				0x0b
#define        	ADRPOSA 				0x0c
#define        	ADRPOSB 				0x0d
#define        	ADIARMSOS     	0x0e
#define        	ADIBRMSOS     	0x0f
#define        	ADIBGain      	0x10
#define					ADD2FPL       	0x11
#define        	ADD2FPH       	0x12
#define        	ADDCIAH       	0x13
#define        	ADDCIBH       	0x14
#define         ADDCUH					0x15   
#define         ADDCL   				0x16 
#define         ADEMUCON2				0x17
#define					ADPFCnt    			0x20
#define        	ADDFcnt    			0x21
#define        	ADIARMS       	0x22
#define        	ADIBRMS       	0x23
#define        	ADURMS        	0x24
#define					ADUFreq       	0x25
#define        	ADPowerPA     	0x26
#define        	ADPowerPB     	0x27
#define         ADEnergyP  			0x29
#define         ADEnergyP2 			0x2a
#define         ADEnergyD  			0x2b
#define         ADEnergyD2    	0x2c
#define         ADEMUStatus   	0x2d
#define         ADSPL_IA      	0x30
#define         ADSPL_IB      	0x31
#define         ADSPL_U       	0x32
#define         ADIE  					0x40
#define         ADIF  					0x41
#define         ADRIF    				0x42
#define         ADSysStatus 	 	0x43
#define         ADRData      		0x44
#define         ADWData      		0x45
#define         ADDeviceID   		0x7f
#define         SpecialReg   		0xea
#define					WriteEn					0xe5
#define					WriteDis				0xdc
//----------------------end RN8209  ��ַ����-----------------------------------------------//

#define SENSOR_READ_COUNT 	250

typedef struct {
	union {
		volatile uint32_t id;
		volatile uint8_t idbuf[4];

	} unid;

	union {
		volatile uint32_t vol;
		volatile uint8_t volbuf[4];
	} unvol;

	union {
		volatile uint32_t curt;
		volatile uint8_t curtbuf[4];
	} uncurt;

	union {
		volatile uint32_t ergy;
		volatile uint8_t ergybuf[4];
	} unergy;

	volatile uint8_t volfailed_time;	
	volatile uint8_t curtfailed_time;	
	volatile uint8_t define_curtfailed_time;	
	volatile uint8_t ergy_offset_enable;
	volatile uint32_t ergy_offsetP;//������
	volatile uint32_t ergy_offsetN;//������
} EnergyDataType;

EnergyDataType EnergyData[MAX_COM_QUANTITY];
sComPack_TypeDef ComPack;

volatile const uint16_t RevRelaySelectionBuf[10]={0xfffd, 0xfffb, 0xfff7, 0xffef, 0xffdf, 0xffbf, 0xff7f, 0xfdff, 0xfbff, 0xf7ff};
// 1        2       3       4       5       6       7       8      9      10       
volatile const uint16_t RelaySelectionBuf[12]=   {0x03,   0x05,   0x09,    0x11,   0x21,   0x41,   0x81,   0x201,  0x401,  0x801, 0xfff,0x0 };
// 1        2      3        4       5       6       7       8      9      10   ȫ��  ȫ��  //bit13-bit16��4λ��Ӧ���ں� 0-3,1-4,2-5,3-2,4-1

volatile const uint16_t ComSelectionBuf[5]={0x4000,0x3000,0x0,0x1000,0x2000};

volatile uint16_t ComRelayStatus = 0;//ͨ����״̬

void HC595OutByte(uint16_t data) {
	uint16_t i;

	//__set_PRIMASK(1); 
	for(i = 0x8000; i>0; i/=2) {
		if(data & i) {
			CSER_HIGH();
		} else {
			CSER_LOW();
		}

		DelayUs(1);
		CSCK_LOW();	
		DelayUs(145);//75//��СֵΪ40
		CSCK_HIGH();
		DelayUs(145);

	}

	CRCK_LOW();
	DelayUs(10);
	CRCK_HIGH();
	DelayUs(1);
	//__set_PRIMASK(0);		
}

void gisunlink_energy_metering_Init(void)
{
	uint8_t i;

	gisunlink_check_usart_init();	
	
	ComRelayStatus = ComSelectionBuf[0]|RelaySelectionBuf[11];//�ر����м̵������,ѡͨ����0
	HC595OutByte(ComRelayStatus);
	COE_LOW();//ʹ��595��Ч
	DelayMs(20);

	for(i=0;i<5;i++)//Calibration all channel
	{
		HC595OutByte(ComSelectionBuf[i]);
		DelayMs(20);
		EMU_Calibration();
		DelayMs(20);
	}
}

/*****************************************************************************
 ** Function name:fnRN8209_Write(u8 wReg,u8 *pBuf,u8 ucLen)
 **
 ** Description:дRN8209�Ĵ���
 **
 ** Parameters:wReg �Ĵ�����ַ��*pBuf��д��ֵ�Ĵ�ŵ�ַ��ucLen����д��ֵ�ĳ���
 **
 ** Returned value:	������ʶ-�ɹ���ʧ��
 **
 ******************************************************************************/

ErrorStatus fnRN8209_Write(u8 wReg,u8 *pBuf,u8 ucLen) {
	u8 i,temp,chksum;

	if( (ucLen == 0) || (ucLen > 4) ) return(ERROR);


	ComPack.pTx=&ComPack.TxBuf[0];	

	//д����ǰ���ȷ��������ֽڣ������ֽڵ����λbit[7]=0:��������1��д������bit[6:0]Ϊ�������Ĵ�ĵ�ַ		
	temp =wReg|0x80;//�������Ĵ�����ַ���λ��1��ʹ�����ֽ�Ϊд����
	*(ComPack.pTx++)=temp;
	chksum = temp;		
	for(i = ucLen; i > 0;i-- )
	{		
		*(ComPack.pTx++)=pBuf[i-1];	//��RN8209��������		
		chksum +=pBuf[i-1];
	}
	chksum = ~chksum;
	*(ComPack.pTx++)=chksum;
	ComPack.TxLen = ucLen+2;
	ComPack.pTx=&ComPack.TxBuf[0];

	for(i=0;i<ComPack.TxLen;i++)//���ڷ���
	{
		USART_SendData(USART3,*(ComPack.pTx++));
		while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); 
	}

	//RN8209дʹ�ܻ�д��������д��WData�Ĵ�����飨�Ƿ���ȷд�룩-----------------------	
	ComPack.pTx=&ComPack.TxBuf[0];	


	return(SUCCESS);
}
/*****************************************************************************  
 ** Function name:fnRN8209_Read(u8 wReg,u8 *pBuf,u8 ucLen)                      
 **                                                                              
 ** Description:��RN8209�Ĵ���                                                   
 **                                                                              
 ** Parameters:wReg �Ĵ�����ַ��*pBuf����ֵ�Ĵ�ŵ�ַ��ucLen������ֵ�ĳ���   
 **                                                                              
 ** Returned value:	������ʶ-�ɹ���ʧ��                                         
 **                                                                              
 ******************************************************************************/ 

ErrorStatus fnRN8209_Read(u8 wReg,u8 *pBuf,u8 ucLen) {
	u8 i,temp;
	u8 j=0;
	u8 chksum=0;	
	ErrorStatus	err;
	if(ucLen == 0) return(ERROR);

	err  = SUCCESS;	    
	chksum=wReg;
	j = 0;	  

	ComPack.TxLen = 1;	
	ComPack.RxLen=0; 

	USART_SendData(USART3,wReg);//����������
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); //wite

	i = SENSOR_READ_COUNT;

	while(i--) {//�ȴ������������
		DelayMs(1);
		if(ComPack.RxLen >= (ucLen+1) )
			break;		
	}

#if 0
	for(i=10;i>0;i--)
	{
		DelayMs(1);
		if(ComPack.RxLen>= (ucLen+1))
			break;
	}
#endif

	for(i = ucLen; i > 0;i--)
	{
		pBuf[i-1] = ComPack.RxBuf[j++]; //�����յ������ݱ��浽����pBuf[]
		chksum += pBuf[i-1];//����������ݵ�У���
	}
	chksum = ~chksum;
	temp=ComPack.RxBuf[j++];

	if(temp!=chksum)  
	{//��У��ʹ������������
		err = ERROR;
	}
	return(err);
}

void EMU_Calibration(void) {

	uint8_t dataLen=0;
	uint8_t writeTempBuf[4];
	//uint8_t readTempBuf[4];

	writeTempBuf[0]=WriteEn;
	fnRN8209_Write(SpecialReg ,writeTempBuf,1);//����дʹ��

	DelayMs(10);
	writeTempBuf[0]=0xfa;
	fnRN8209_Write(SpecialReg ,writeTempBuf,1);
	
	DelayMs(10);
	writeTempBuf[0]=WriteEn;
	fnRN8209_Write(SpecialReg ,writeTempBuf,1);//����дʹ��	
	
	writeTempBuf[0]=0x51;
	writeTempBuf[1]=0x16;//SYSCON ����ͨ��B����������A,B,����Ϊ2�� ;��ѹ����Ϊ1
	dataLen=2;
	fnRN8209_Write(ADSYSCON,writeTempBuf,dataLen);//д

	writeTempBuf[0]=0x03;
	writeTempBuf[1]=0x28;//
	dataLen=2;
	fnRN8209_Write(ADEMUCON ,writeTempBuf,dataLen);//д

	writeTempBuf[0]=0xf0;//D0˫ͨ������
	writeTempBuf[1]=0x00;//�����ٶ�13.982hz
	dataLen=2;
	fnRN8209_Write(ADEMUCON2,writeTempBuf,dataLen);//д

	writeTempBuf[0]=0x91;
	writeTempBuf[1]=0x02;//
	dataLen=2;
	fnRN8209_Write(ADHFConst ,writeTempBuf,dataLen);//д	

	writeTempBuf[0]=0xff;
	writeTempBuf[1]=0xfe;//A��������
	dataLen=2;
	fnRN8209_Write(ADIARMSOS,writeTempBuf,dataLen);//д	

	writeTempBuf[0]=0xff;
	writeTempBuf[1]=0xfe;//B��������
	dataLen=2;
	fnRN8209_Write(ADIBRMSOS,writeTempBuf,dataLen);//д	

	writeTempBuf[0]=WriteDis;
	fnRN8209_Write(SpecialReg ,writeTempBuf,1);//�ر�дʹ��

}

uint8_t gisunlink_open_check_channel(uint8_t channel) {

	uint8_t chk_try = 3; uint8_t ret = 1;	
	EnergyDataType *comEnergy = NULL;
	ComRelayStatus |= RelaySelectionBuf[channel];
	HC595OutByte(ComRelayStatus);

	if(channel>9) {
		return !ret;
	}	else {
		comEnergy = &EnergyData[channel];
		memset(comEnergy,0x00,sizeof(EnergyDataType));		
	}			

	while(chk_try--) {
		ret = fnRN8209_Read(0x7f,(uint8_t *)comEnergy->unid.idbuf,3);
		if(comEnergy->unid.id == 0x00820900 && ret == SUCCESS) {
			comEnergy->ergy_offset_enable = 1;
			break;
		}
		DelayMs(20);	
	}
	return !ret;
}

void gisunlink_close_check_channel(uint8_t channel) {
	ComRelayStatus &= RevRelaySelectionBuf[channel];
	HC595OutByte(ComRelayStatus);	
	//gisunlink_read_energy_data();
	DelayMs(2);		
}

uint8_t gisunlink_read_channel_vol(uint8_t reg,EnergyDataType *comEnergy,uint8_t len) {
	uint8_t chk_try = 3;
	uint8_t ret = 0;		
	while(chk_try--) {
		if((ret = fnRN8209_Read(reg,(uint8_t *)comEnergy->unvol.volbuf,len)) == SUCCESS) {
			ret = ERROR;					//100v-264v
			if((comEnergy->unvol.vol > 550000) && (comEnergy->unvol.vol < 1450000)) {
				ret = SUCCESS;
				break;				
			}
		}
	}	
	return ret;
}

uint8_t gisunlink_read_channel_curt(uint8_t reg,EnergyDataType *comEnergy,uint8_t len) {
	uint8_t chk_try = 3;
	uint8_t ret = 0;		
	while(chk_try--) {
		if((ret = fnRN8209_Read(reg,(uint8_t *)comEnergy->uncurt.curtbuf,len)) == SUCCESS) {
			ret = ERROR;
			if(comEnergy->uncurt.curt < 200000) {		//50000 3A
				ret = SUCCESS;
				break;				
			}
		}
	}	
	return ret;
}

uint8_t gisunlink_read_channel_ergy(uint8_t reg,EnergyDataType *comEnergy,uint8_t len) {
	uint8_t chk_try = 3;
	uint8_t ret = 0;	
	while(chk_try--) {
		if((ret = fnRN8209_Read(reg,(uint8_t *)comEnergy->unergy.ergybuf,len)) == SUCCESS) {
			break;
		}
	}
	return ret;	
}

union {
	uint32_t readTemp;
	uint8_t readTempBuf[4];
} unread;

//��ȡͨ������
static uint8_t gisunlink_read_energy_data(charge_com *com,gisunlink *global) {
	
	uint8_t c_reg = ADIARMS;
	uint8_t e_reg = ADEnergyP;
	EnergyDataType *comEnergy = NULL;

	if(com == NULL || com->id > 9) {
		return 1;
	} 

	comEnergy = &EnergyData[com->id];

	//ѡ��ͨ��B
	if((com->id&0x01) == 1) {
		c_reg = ADIBRMS;
		e_reg = ADEnergyD;
	}	

	ComRelayStatus &=0x0fff;//����comλ
	ComRelayStatus |= ComSelectionBuf[com->id/2];//ѡ��comλ
	HC595OutByte(ComRelayStatus);	

	DelayMs(1);	

	fnRN8209_Read(ADEMUStatus,unread.readTempBuf,3);
	
	if((unread.readTemp&0x0000ffff) != 0xbeac) {
		EMU_Calibration();
		uint16_t icReset = global->icReset[com->id];
		icReset++;
		global->icReset[com->id] = icReset;
		comEnergy->ergy_offsetP = com->used*16/5;
		comEnergy->ergy_offsetN = 0;

		if((com->id&0x01) == 1)//��ǰΪ�ڶ�ͨ��
		{
			comEnergy = &EnergyData[com->id - 1];
			
		}
		else//��ǰΪ��һͨ��
		{
			comEnergy = &EnergyData[com->id + 1];
		}

		comEnergy->ergy_offsetP = com->used*16/5;
		comEnergy->ergy_offsetN = 0;
		comEnergy = &EnergyData[com->id];//�л�����ǰ��ַ
		
		DelayMs(1);
		return SUCCESS;
	}

	if(comEnergy->unid.id == 0x00820900) {

		if(gisunlink_read_channel_ergy(e_reg,comEnergy,3) == SUCCESS) {//����		
			if(comEnergy->ergy_offset_enable == 1) {
				comEnergy->ergy_offset_enable = 0;
				comEnergy->ergy_offsetP = 0;
				comEnergy->ergy_offsetN = comEnergy->unergy.ergy;
			}			
			com->used = (abs((int)(comEnergy->unergy.ergy - comEnergy->ergy_offsetN)) * 5 / 16) + comEnergy->ergy_offsetP;
		} 

		if(gisunlink_read_channel_vol(ADURMS,comEnergy,3) != SUCCESS) { //��ѹ
			if(comEnergy->volfailed_time == 0) {
				comEnergy->volfailed_time = com->utime;
			} else {
				if(com->utime > comEnergy->volfailed_time && (com->utime - comEnergy->volfailed_time) >= 5) {						
					comEnergy->volfailed_time = 0;								
					com->breakdown_time = 0;
					com->breakdown |= VOLTAGE_OVERFLOW;													//��ѹ���ڷ�Χ
					//gisunlink_close_check_channel(com->id);														
					return ERROR;	
				}
			}
		} else {
			comEnergy->volfailed_time = 0;	
			com->breakdown &= ~(VOLTAGE_OVERFLOW);				
		}

		if(gisunlink_read_channel_curt(c_reg,comEnergy,3) != SUCCESS) { //��ȡ����
			if(comEnergy->curtfailed_time == 0) {
				comEnergy->curtfailed_time = com->utime;
			} else {
				if(com->utime > comEnergy->curtfailed_time && (com->utime - comEnergy->curtfailed_time) >= 5) {			
					comEnergy->curtfailed_time = 0;					
					com->breakdown_time = 0;
					com->breakdown |= CURREN_OVERFLOW_3A;													//��������3A
					//gisunlink_close_check_channel(com->id);	
					return ERROR;		
				}
			}
		} else {
			comEnergy->curtfailed_time = 0;
			com->breakdown &= ~(CURREN_OVERFLOW_3A);
			com->cur_current = comEnergy->uncurt.curt*4/66;//MA							
		}

		if(com->cur_current <= com->max_current ) { 
			comEnergy->define_curtfailed_time = 0;
			com->breakdown &= ~(CURREN_OVERFLOW_SETTING);			
		}	else {				
			if(comEnergy->define_curtfailed_time == 0) {
				comEnergy->define_curtfailed_time = com->utime;
			} else {
				if(com->utime > comEnergy->define_curtfailed_time && (com->utime - comEnergy->define_curtfailed_time) >= 5) {			
					comEnergy->define_curtfailed_time = 0;									
					com->breakdown_time = 0;
					com->breakdown |= CURREN_OVERFLOW_SETTING;										//�������õĵ���
					//gisunlink_close_check_channel(com->id);	
					return ERROR;								
				}
			}				
		}		
	} 
	return SUCCESS;	
}

//��ȡ��ǰ�ļ�������
void gisunlink_read_realtime_meter_data(gisunlink *global) {
	static uint8_t read_comID = 0;
	LED_Toggle();			
	
//	if(global->comList[read_comID].enable && global->comList[read_comID].breakdown == NO_BREAKDOWN) {	
		gisunlink_read_energy_data(&global->comList[read_comID],global);
//	} else {
//		gisunlink_close_check_channel(read_comID);	
//	}
	
	read_comID++; 
	if(read_comID >= MAX_COM_QUANTITY) {
		read_comID = 0;
	}	
}
