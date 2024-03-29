#include "gisunlink_eeprom.h"
#include "string.h"
#



#define ACCHK					GPIO_Pin_4
#define ACCHK_PORT              GPIOB
#define ACCHK_CLK               RCC_APB2Periph_GPIOB
#define	ACCHK_HIGH()			ACCHK_PORT->BSRR = ACCHK
#define	ACCHK_LOW()				ACCHK_PORT->BRR = ACCHK


#define I2C						GPIOB			
#define RCC_I2C_PORT 			RCC_APB2Periph_GPIOB	
#define I2C_SCL_PIN				GPIO_Pin_9			
#define I2C_SDA_PIN				GPIO_Pin_8			

/* 定义读写SCL和SDA的宏 */
#define I2C_SCL_1()  			I2C->BSRR = I2C_SCL_PIN				/* SCL = 1 */
#define I2C_SCL_0()  			I2C->BRR = I2C_SCL_PIN				/* SCL = 0 */

#define I2C_SDA_1()  			I2C->BSRR = I2C_SDA_PIN				/* SDA = 1 */
#define I2C_SDA_0() 			I2C->BRR = I2C_SDA_PIN				/* SDA = 0 */

#define I2C_SDA_READ()  		((I2C->IDR & I2C_SDA_PIN) != 0)	/* 读SDA口线状态 */
#define I2C_SCL_READ()  		((I2C->IDR & I2C_SCL_PIN) != 0)	/* 读SCL口线状态 */
				 

void gisunlink_e2prom_Init(void)
{

	GPIO_InitTypeDef GPIO_Init_Structure;
	RCC_APB2PeriphClockCmd(RCC_I2C_PORT,ENABLE);
	GPIO_Init_Structure.GPIO_Pin = I2C_SCL_PIN | I2C_SDA_PIN;
	GPIO_Init_Structure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_Init_Structure);
	GPIO_SetBits(I2C,I2C_SCL_PIN | I2C_SDA_PIN);

	/* 给一个停止信号, 复位I2C总线上的所有设备到待机模式 */
	i2c_Stop();

}


/*
*********************************************************************************************************
*	函 数 名: i2c_Delay
*	功能说明: I2C总线位延迟，最快400KHz
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void i2c_Delay(void)
{
		uint8_t i,j;	
	for(i=0;i<10;i++)
		for(j=10;j>0; j--);
}

/*
*********************************************************************************************************
*	函 数 名: i2c_Start
*	功能说明: CPU发起I2C总线启动信号
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_Start(void)
{
	/* 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号 */
	I2C_SDA_1();
	I2C_SCL_1();
	i2c_Delay();
	I2C_SDA_0();
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();
}

/*
*********************************************************************************************************
*	函 数 名: i2c_Start
*	功能说明: CPU发起I2C总线停止信号
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_Stop(void)
{
	/* 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号 */
	I2C_SDA_0();
	I2C_SCL_1();
	i2c_Delay();
	I2C_SDA_1();
}

/*
*********************************************************************************************************
*	函 数 名: i2c_SendByte
*	功能说明: CPU向I2C总线设备发送8bit数据
*	形    参:  _ucByte ： 等待发送的字节
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_SendByte(uint8_t _ucByte)
{
	uint8_t i;

	/* 先发送字节的高位bit7 */
	for (i = 0; i < 8; i++)
	{
		if (_ucByte & 0x80)
		{
			I2C_SDA_1();
		}
		else
		{
			I2C_SDA_0();
		}
		i2c_Delay();
		I2C_SCL_1();
		i2c_Delay();
		I2C_SCL_0();
		if (i == 7)
		{
			 I2C_SDA_1(); // 释放总线
		}
		_ucByte <<= 1;	/* 左移一个bit */
		i2c_Delay();
	}
}

/*
*********************************************************************************************************
*	函 数 名: i2c_ReadByte
*	功能说明: CPU从I2C总线设备读取8bit数据
*	形    参:  无
*	返 回 值: 读到的数据
*********************************************************************************************************
*/
uint8_t i2c_ReadByte(void)
{
	uint8_t i;
	uint8_t value;

	/* 读到第1个bit为数据的bit7 */
	value = 0;
	for (i = 0; i < 8; i++)
	{
		value <<= 1;
		I2C_SCL_1();
		i2c_Delay();
		if (I2C_SDA_READ())
		{
			value++;
		}
		I2C_SCL_0();
		i2c_Delay();
	}
	return value;
}

/*
*********************************************************************************************************
*	函 数 名: i2c_WaitAck
*	功能说明: CPU产生一个时钟，并读取器件的ACK应答信号
*	形    参:  无
*	返 回 值: 返回0表示正确应答，1表示无器件响应
*********************************************************************************************************
*/
uint8_t i2c_WaitAck(void)
{
	uint8_t re;

	I2C_SDA_1();	/* CPU释放SDA总线 */
	i2c_Delay();
	I2C_SCL_1();	/* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
	i2c_Delay();
	if (I2C_SDA_READ())	/* CPU读取SDA口线状态 */
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	I2C_SCL_0();
	i2c_Delay();
	return re;
}

/*
*********************************************************************************************************
*	函 数 名: i2c_Ack
*	功能说明: CPU产生一个ACK信号
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_Ack(void)
{
	I2C_SDA_0();	/* CPU驱动SDA = 0 */
	i2c_Delay();
	I2C_SCL_1();	/* CPU产生1个时钟 */
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();
	I2C_SDA_1();	/* CPU释放SDA总线 */
}

/*
*********************************************************************************************************
*	函 数 名: i2c_NAck
*	功能说明: CPU产生1个NACK信号
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_NAck(void)
{
	I2C_SDA_1();	/* CPU驱动SDA = 1 */
	i2c_Delay();
	I2C_SCL_1();	/* CPU产生1个时钟 */
	i2c_Delay();
	I2C_SCL_0();
	i2c_Delay();
}

/*
*********************************************************************************************************
*	函 数 名   : i2c_CheckDevice
*	功能说明: 检测I2C总线设备，CPU向发送设备地址，然后读取设备应答来判断该设备是否存在
*	形    参       :  _Address：设备的I2C总线地址
*	返 回 值    : 返回值 0 表示正确， 返回1表示未探测到
*********************************************************************************************************
*/
uint8_t i2c_CheckDevice(uint8_t _Address)
{
	uint8_t ucAck;

	if (I2C_SDA_READ() && I2C_SCL_READ())
	{
		i2c_Start();		/* 发送启动信号 */

		/* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
		i2c_SendByte(_Address | I2C_WR);
		ucAck = i2c_WaitAck();	/* 检测设备的ACK应答 */

		i2c_Stop();			/* 发送停止信号 */

		return ucAck;
	}
	return 1;	/* I2C总线异常 */
}


uint8_t ee_CheckOk(void)
{
	if (i2c_CheckDevice(EE_DEV_ADDR) == 0)
	{
		return 1;
	}
	else
	{
		/* 失败后，切记发送I2C总线停止信号 */
		i2c_Stop();
		return 0;
	}
}


/*
*********************************************************************************************************
*	函 数 名   : ee_ReadBytes
*	功能说明: 从串行EEPROM指定地址处开始读取若干数据
*	形    参      :  _usAddress : 起始地址
*			          _usSize : 数据长度，单位为字节
*			          _pReadBuf : 存放读到的数据的缓冲区指针
*	返 回 值  : 10 表示失败，0表示成功
*********************************************************************************************************
*/
uint8_t ee_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize)
{
	uint16_t i;

	/* 采用串行EEPROM随即读取指令序列，连续读取若干字节 */

	/* 第1步：发起I2C总线启动信号 */
	i2c_Start();

	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	i2c_SendByte(EE_DEV_ADDR | I2C_WR);	/* 此处是写指令 */

	/* 第3步：发送ACK */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
	if (EE_ADDR_BYTES == 1)
	{
		i2c_SendByte((uint8_t)_usAddress);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	/* EEPROM器件无应答 */
		}
	}
	else
	{
		i2c_SendByte(_usAddress >> 8);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	/* EEPROM器件无应答 */
		}

		i2c_SendByte(_usAddress);
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	/* EEPROM器件无应答 */
		}
	}

	/* 第6步：重新启动I2C总线。下面开始读取数据 */
	i2c_Start();

	/* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	i2c_SendByte(EE_DEV_ADDR | I2C_RD);	/* 此处是读指令 */

	/* 第8步：发送ACK */
	if (i2c_WaitAck() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}

	/* 第9步：循环读取数据 */
	for (i = 0; i < _usSize; i++)
	{
		_pReadBuf[i] = i2c_ReadByte();	/* 读1个字节 */

		/* 每读完1个字节后，需要发送Ack， 最后一个字节不需要Ack，发Nack */
		if (i != _usSize - 1)
		{
			i2c_Ack();	/* 中间字节读完后，CPU产生ACK信号(驱动SDA = 0) */
		}
		else
		{
			i2c_NAck();	/* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
		}
	}
	/* 发送I2C总线停止信号 */
	i2c_Stop();
	return 0;	/* 执行成功 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	i2c_Stop();
	return 1;
}

/*
*********************************************************************************************************
*	函 数 名   : ee_WriteBytes
*	功能说明: 向串行EEPROM指定地址写入若干数据，采用页写操作提高写入效率
*	形    参      :  _usAddress : 起始地址
*			          _usSize : 数据长度，单位为字节
*			          _pWriteBuf : 存放读到的数据的缓冲区指针
*	返 回 值  : 1 表示失败，0表示成功
*********************************************************************************************************
*/
uint8_t ee_WriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize)
{
	uint16_t i,m;
	uint16_t usAddr;

	/*
		写串行EEPROM不像读操作可以连续读取很多字节，每次写操作只能在同一个page。
		对于24xx02，page size = 8
		简单的处理方法为：按字节写操作模式，每写1个字节，都发送地址
		为了提高连续写的效率: 本函数采用page wirte操作。
	*/

	usAddr = _usAddress;
	for (i = 0; i < _usSize; i++)
	{
		/* 当发送第1个字节或是页面首地址时，需要重新发起启动信号和地址 */
		if ((i == 0) || (usAddr & (EE_PAGE_SIZE - 1)) == 0)
		{
			/*　第０步：发停止信号，启动内部写操作　*/
			i2c_Stop();

			/* 通过检查器件应答的方式，判断内部写操作是否完成, 一般小于 10ms
				CLK频率为200KHz时，查询次数为30次左右
			*/
			for (m = 0; m < 1000; m++)
			{
				/* 第1步：发起I2C总线启动信号 */
				i2c_Start();

				/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
				i2c_SendByte(EE_DEV_ADDR | I2C_WR);	/* 此处是写指令 */

				/* 第3步：发送一个时钟，判断器件是否正确应答 */
				if (i2c_WaitAck() == 0)
				{
					break;
				}
			}
			if (m  == 1000)
			{
				goto cmd_fail;	/* EEPROM器件写超时 */
			}

			/* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
			if (EE_ADDR_BYTES == 1)
			{
				i2c_SendByte((uint8_t)usAddr);
				if (i2c_WaitAck() != 0)
				{
					goto cmd_fail;	/* EEPROM器件无应答 */
				}
			}
			else
			{
				i2c_SendByte(usAddr >> 8);
				if (i2c_WaitAck() != 0)
				{
					goto cmd_fail;	/* EEPROM器件无应答 */
				}

				i2c_SendByte(usAddr);
				if (i2c_WaitAck() != 0)
				{
					goto cmd_fail;	/* EEPROM器件无应答 */
				}
			}
		}

		/* 第6步：开始写入数据 */
		i2c_SendByte(_pWriteBuf[i]);

		/* 第7步：发送ACK */
		if (i2c_WaitAck() != 0)
		{
			goto cmd_fail;	/* EEPROM器件无应答 */
		}

		usAddr++;	/* 地址增1 */
	}

	/* 命令执行成功，发送I2C总线停止信号 */
	i2c_Stop();
	return 0;

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	i2c_Stop();
	return 1;
}



/********************************************************************
*
* Name:		uchar Write_Data(uint addr,uchar len,uchar idata *buff)
* Function: write data into eeprom,24c65/24c128/24c256
* In:		uint addr -- write data start address
* 			uchar len -- write data length
*			uchar *buff -- data to be write
* Result:	OK -- write data OK
*			WriteData_Err -- write data error
*
********************************************************************/
// u8 Write_Data(u16 addr,u16 len,u8 *buff)
// {
// //   u8 buf[128];
//    u8 err1;

//   err1 = ee_WriteBytes(buff, addr, len);

//   return err1;
//   
// #if 0
//  // for(read_addr=0;read_addr<len;read_addr++)
// //   printf("%d ",buff[read_addr]);
//   DelayMs(2);
//    //400K时，最少延时Delay(38000);
//    
//    //Delay(38000);
//   ee_ReadBytes(buf, addr, len);
//  // for(read_addr=0;read_addr<len;read_addr++)
// //   printf("%d ",buf[read_addr]);

//   if (memcmp(buf,buff,len)==0)
//   {
// 	return 0;
//   }
//   else
//   {
// 	return 1;
//   }
// #endif

// }

// /********************************************************************
// *
// * Name:		uchar Read_Data(uint addr,uchar len,uchar *buff)
// * Function: read data from eeprom,24c65/24c128/24c256
// * In:		uint addr -- read data start address
// * 			uchar len -- read data length
// *			uchar *buff -- read data buffer
// * Result:	OK -- read data OK
// *			ReadData_Err -- read data error
// *
// ********************************************************************/
// u8 Read_Data(u16 addr,u16 len,u8 *buff)
// {
//    u8 err1;
//   //F25CFXX_Read(addr,len,buff);
//   err1 = ee_ReadBytes(buff, addr, len);

//   return err1;
// }


