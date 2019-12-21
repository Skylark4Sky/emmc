#include "stm32f10x.h"
#include "Lcd_Driver.h"
#include "gisunlink_delay.h"
#include "gisunlink_delay.h"


/***************************************************************************************
STM32测试平台介绍:
开发板：正点原子MiniSTM32开发板
MCU ：STM32_F103_RBT6
晶振 ：12MHZ
主频 ：72MHZ
接线说明:
//-------------------------------------------------------------------------------------
#define LCD_CTRL   	  	GPIOB		//定义TFT数据端口
#define LCD_LED        	GPIO_Pin_9  //PB9 连接至TFT -LED
#define LCD_RS         	GPIO_Pin_10	//PB10连接至TFT --RS
#define LCD_CS        	GPIO_Pin_11 //PB11 连接至TFT --CS
#define LCD_RST     	GPIO_Pin_12	//PB12连接至TFT --RST
#define LCD_SCL        	GPIO_Pin_13	//PB13连接至TFT -- CLK
#define LCD_SDA        	GPIO_Pin_15	//PB15连接至TFT - SDI
//VCC:可以接5V也可以接3.3V
//LED:可以接5V也可以接3.3V或者使用任意空闲IO控制(高电平使能)
//GND：接电源地
//说明：如需要尽可能少占用IO，可以将LCD_CS接地，LCD_LED接3.3V，LCD_RST接至单片机复位端，
//将可以释放3个可用IO
//接口定义在Lcd_Driver.h内定义，
//如需变更IO接法，请根据您的实际接线修改相应IO初始化LCD_GPIO_Init()
//-----------------------------------------------------------------------------------------
例程功能说明：
1.	简单刷屏测试
2.	英文显示测试示例
3.	中文显示测试示例
4.	数码管字体显示示例
5.	图片显示示例
6.	2D按键菜单示例
7.	本例程支持横屏/竖屏切换(开启宏USE_HORIZONTAL,详见Lcd_Driver.h)
8.	本例程支持软件模拟SPI/硬件SPI切换(开启宏USE_HARDWARE_SPI,详见Lcd_Driver.h)
**********************************************************************************************/


//---------------------------------function----------------------------------------------------//

/****************************************************************************
* 名    称：void LCD_GPIO_Init(void)
* 功    能：STM32_模拟SPI所用到的GPIO初始化
* 入口参数：无
* 出口参数：无
* 说    明：初始化模拟SPI所用的GPIO
****************************************************************************/
void LCD_GPIO_Init(void) {
	GPIO_InitTypeDef  GPIO_InitStructure;
	      
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA ,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB ,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0| GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
/****************************************************************************
* 名    称：void  SPIv_WriteData(u8 Data)
* 功    能：STM32_模拟SPI写一个字节数据底层函数
* 入口参数：Data
* 出口参数：无
* 说    明：STM32_模拟SPI读写一个字节数据底层函数
****************************************************************************/
void  SPIv_WriteData(u8 Data)
{
	unsigned char i=0;
	for(i=8;i>0;i--)
	{
		if(Data&0x80)	
	  LCD_SDA_SET; //输出数据
      else LCD_SDA_CLR;
	   
      LCD_SCL_CLR;       
      LCD_SCL_SET;
      Data<<=1; 
	}
}

/****************************************************************************
* 名    称：Lcd_WriteIndex(u8 Index)
* 功    能：向液晶屏写一个8位指令
* 入口参数：Index   寄存器地址
* 出口参数：无
* 说    明：调用前需先选中控制器，内部函数
****************************************************************************/
void Lcd_WriteIndex(u8 Index)
{
   LCD_CS_CLR;
   LCD_RS_CLR;
   SPIv_WriteData(Index);
   LCD_CS_SET;
}

/****************************************************************************
* 名    称：Lcd_WriteData(u8 Data)
* 功    能：向液晶屏写一个8位数据
* 入口参数：dat     寄存器数据
* 出口参数：无
* 说    明：向控制器指定地址写入数据，内部函数
****************************************************************************/
void Lcd_WriteData(u8 Data)
{
   LCD_CS_CLR;
   LCD_RS_SET;
   SPIv_WriteData(Data);
   LCD_CS_SET;
}

/****************************************************************************
* 名    称：void LCD_WriteReg(u8 Index,u16 Data)
* 功    能：写寄存器数据
* 入口参数：Index,Data
* 出口参数：无
* 说    明：本函数为组合函数，向Index地址的寄存器写入Data值
****************************************************************************/
void LCD_WriteReg(u8 Index,u16 Data)
{
	Lcd_WriteIndex(Index);
  Lcd_WriteData_16Bit(Data);
}

/****************************************************************************
* 名    称：void Lcd_WriteData_16Bit(u16 Data)
* 功    能：向液晶屏写一个16位数据
* 入口参数：Data
* 出口参数：无
* 说    明：向控制器指定地址写入一个16位数据
****************************************************************************/
void Lcd_WriteData_16Bit(u16 Data)
{	
	Lcd_WriteData(Data>>8);
	Lcd_WriteData(Data);	
}

/****************************************************************************
* 名    称：void Lcd_Reset(void)
* 功    能：液晶硬复位函数
* 入口参数：无
* 出口参数：无
* 说    明：液晶初始化前需执行一次复位操作
****************************************************************************/
void Lcd_Reset(void)
{
	LCD_RST_CLR;
	DelayMs(100);
	LCD_RST_SET;
	DelayMs(50);
}
/****************************************************************************
* 名    称：void Lcd_Init(void)
* 功    能：液晶初始化函数
* 入口参数：无
* 出口参数：无
* 说    明：液晶初始化_ILI9225_176X220
****************************************************************************/
void Lcd_Init(void) {	
  LCD_GPIO_Init();
  Lcd_Reset(); //Reset before LCD Init.
	#if 1
  //************* Start Initial Sequence **********//
  Lcd_WriteIndex(0x01); Lcd_WriteData_16Bit(0x001C); // set the display line number and display direction  
  Lcd_WriteIndex(0x02); Lcd_WriteData_16Bit(0x0100); // set 1 line inversion
  Lcd_WriteIndex(0x03); Lcd_WriteData_16Bit(0x10A8); // set GRAM write direction .1038
  Lcd_WriteIndex(0x08); Lcd_WriteData_16Bit(0x0808); // set BP and FP
  Lcd_WriteIndex(0x0B); Lcd_WriteData_16Bit(0x1100); //frame cycle
  Lcd_WriteIndex(0x0C); Lcd_WriteData_16Bit(0x0000); // RGB interface setting R0Ch=0x0110 for RGB 18Bit and R0Ch=0111for RGB16Bit
  Lcd_WriteIndex(0x0F); Lcd_WriteData_16Bit(0x0001); //1401	// Set frame rate----0801
  //Lcd_WriteIndex(0x15); Lcd_WriteData_16Bit(0x0000); //set system interface
  Lcd_WriteIndex(0x20); Lcd_WriteData_16Bit(0x0000); // Set GRAM Address
  Lcd_WriteIndex(0x21); Lcd_WriteData_16Bit(0x0000); // Set GRAM Address
  //*************Power On sequence ****************//
  Lcd_WriteIndex(0x10); Lcd_WriteData_16Bit(0x0000); 	//0800	// Set SAP,DSTB,STB----0A00
  Lcd_WriteIndex(0x11); Lcd_WriteData_16Bit(0x1B41);  //1F3F	// Set APON,PON,AON,VCI1EN,VC----1038
  Lcd_WriteIndex(0x12); Lcd_WriteData_16Bit(0x200E);  //0121	// Internal reference voltage= Vci;----1121
  Lcd_WriteIndex(0x13); Lcd_WriteData_16Bit(0x0052);  //006F	// Set GVDD----0066
  Lcd_WriteIndex(0x14); Lcd_WriteData_16Bit(0x4B5C);  //4349 	// Set VCOMH/VCOML voltage----5F60
  DelayMs(100);                         // Delay 50ms
  //-------------- Set GRAM area -----------------//
  Lcd_WriteIndex(0x30); Lcd_WriteData_16Bit(0x0000);
  Lcd_WriteIndex(0x31); Lcd_WriteData_16Bit(0x00DB);
  Lcd_WriteIndex(0x32); Lcd_WriteData_16Bit(0x0000);
  Lcd_WriteIndex(0x33); Lcd_WriteData_16Bit(0x0000);
  Lcd_WriteIndex(0x34); Lcd_WriteData_16Bit(0x00DB);
  Lcd_WriteIndex(0x35); Lcd_WriteData_16Bit(0x0000);
  Lcd_WriteIndex(0x36); Lcd_WriteData_16Bit(0x00AF);
  Lcd_WriteIndex(0x37); Lcd_WriteData_16Bit(0x0000);
  Lcd_WriteIndex(0x38); Lcd_WriteData_16Bit(0x00DB);
  Lcd_WriteIndex(0x39); Lcd_WriteData_16Bit(0x0000);
  DelayMs(10);                         // Delay 50ms	
  Lcd_WriteIndex(0xff); Lcd_WriteData_16Bit(0x0003);	
  // ----------- Adjust the Gamma Curve ----------//

  Lcd_WriteIndex(0x50); Lcd_WriteData_16Bit(0x0000);  //0001
  Lcd_WriteIndex(0x51); Lcd_WriteData_16Bit(0x0300);  //200B
  Lcd_WriteIndex(0x52); Lcd_WriteData_16Bit(0x0103);  //0000
  Lcd_WriteIndex(0x53); Lcd_WriteData_16Bit(0x2011);  //0404

  Lcd_WriteIndex(0x54); Lcd_WriteData_16Bit(0x0703);  //0c0c
  Lcd_WriteIndex(0x55); Lcd_WriteData_16Bit(0x0000);  //000c
  Lcd_WriteIndex(0x56); Lcd_WriteData_16Bit(0x0400);  //0101
  Lcd_WriteIndex(0x57); Lcd_WriteData_16Bit(0x0107);  //0400
  Lcd_WriteIndex(0x58); Lcd_WriteData_16Bit(0x2011);  //1108
  Lcd_WriteIndex(0x59); Lcd_WriteData_16Bit(0x0703);  //050C
  Lcd_WriteIndex(0xB0); Lcd_WriteData_16Bit(0x1d01);
  Lcd_WriteIndex(0xFF); Lcd_WriteData_16Bit(0x0000);	
  Lcd_WriteIndex(0x07); Lcd_WriteData_16Bit(0x1017);
  DelayMs(50);  
	#else
  Lcd_WriteIndex(0x02); Lcd_WriteData_16Bit(0x0100); // set 1 line inversion

	//R01H:SM=0,GS=0,SS=0 (for details,See the datasheet of ILI9225)
	LCD_WriteReg(0x01, 0x001C); // set the display line number and display direction
	//R03H:BGR=1,ID0=1,ID1=1,AM=1 (for details,See the datasheet of ILI9225)
	LCD_WriteReg(0x03, 0x1038); // set GRAM write direction .

  Lcd_WriteIndex(0x08); Lcd_WriteData_16Bit(0x0808); // set BP and FP
  Lcd_WriteIndex(0x0B); Lcd_WriteData_16Bit(0x1100); //frame cycle
  Lcd_WriteIndex(0x0C); Lcd_WriteData_16Bit(0x0000); // RGB interface setting R0Ch=0x0110 for RGB 18Bit and R0Ch=0111for RGB16Bit
  Lcd_WriteIndex(0x0F); Lcd_WriteData_16Bit(0x1401); // Set frame rate----0801
  Lcd_WriteIndex(0x15); Lcd_WriteData_16Bit(0x0000); //set system interface
  Lcd_WriteIndex(0x20); Lcd_WriteData_16Bit(0x0000); // Set GRAM Address
  Lcd_WriteIndex(0x21); Lcd_WriteData_16Bit(0x0000); // Set GRAM Address
  //*************Power On sequence ****************//
  DelayMs(50);                         // Delay 50ms
  Lcd_WriteIndex(0x10); Lcd_WriteData_16Bit(0x0800); // Set SAP,DSTB,STB----0A00
  Lcd_WriteIndex(0x11); Lcd_WriteData_16Bit(0x1F3F); // Set APON,PON,AON,VCI1EN,VC----1038
  DelayMs(50);                         // Delay 50ms
  Lcd_WriteIndex(0x12); Lcd_WriteData_16Bit(0x0121); // Internal reference voltage= Vci;----1121
  Lcd_WriteIndex(0x13); Lcd_WriteData_16Bit(0x006F); // Set GVDD----0066
  Lcd_WriteIndex(0x14); Lcd_WriteData_16Bit(0x4349); // Set VCOMH/VCOML voltage----5F60
  //-------------- Set GRAM area -----------------//
  Lcd_WriteIndex(0x30); Lcd_WriteData_16Bit(0x0000);
  Lcd_WriteIndex(0x31); Lcd_WriteData_16Bit(0x00DB);
  Lcd_WriteIndex(0x32); Lcd_WriteData_16Bit(0x0000);
  Lcd_WriteIndex(0x33); Lcd_WriteData_16Bit(0x0000);
  Lcd_WriteIndex(0x34); Lcd_WriteData_16Bit(0x00DB);
  Lcd_WriteIndex(0x35); Lcd_WriteData_16Bit(0x0000);
  Lcd_WriteIndex(0x36); Lcd_WriteData_16Bit(0x00AF);
  Lcd_WriteIndex(0x37); Lcd_WriteData_16Bit(0x0000);
  Lcd_WriteIndex(0x38); Lcd_WriteData_16Bit(0x00DB);
  Lcd_WriteIndex(0x39); Lcd_WriteData_16Bit(0x0000);
  // ----------- Adjust the Gamma Curve ----------//
  Lcd_WriteIndex(0x50); Lcd_WriteData_16Bit(0x0001);  //0400
  Lcd_WriteIndex(0x51); Lcd_WriteData_16Bit(0x200B);  //060B
  Lcd_WriteIndex(0x52); Lcd_WriteData_16Bit(0x0000);  //0C0A
  Lcd_WriteIndex(0x53); Lcd_WriteData_16Bit(0x0404);  //0105
  Lcd_WriteIndex(0x54); Lcd_WriteData_16Bit(0x0C0C);  //0A0C
  Lcd_WriteIndex(0x55); Lcd_WriteData_16Bit(0x000C);  //0B06
  Lcd_WriteIndex(0x56); Lcd_WriteData_16Bit(0x0101);  //0004
  Lcd_WriteIndex(0x57); Lcd_WriteData_16Bit(0x0400);  //0501
  Lcd_WriteIndex(0x58); Lcd_WriteData_16Bit(0x1108);  //0E00
  Lcd_WriteIndex(0x59); Lcd_WriteData_16Bit(0x050C);  //000E
  DelayMs(50);                                     // Delay 50ms
  Lcd_WriteIndex(0x07); Lcd_WriteData_16Bit(0x1017);
  Lcd_WriteIndex(0x22);	
	#endif
}

/*************************************************
函数名：LCD_Set_XY
功能：设置lcd显示起始点
入口参数：xy坐标
返回值：无
*************************************************/
void Lcd_SetXY(u16 Xpos, u16 Ypos)
{
	LCD_WriteReg(0x21,X_MAX_PIXEL - Xpos);
	LCD_WriteReg(0x20,Y_MAX_PIXEL - Ypos);

	Lcd_WriteIndex(0x22);		
} 
/*************************************************
函数名：LCD_Set_Region
功能：设置lcd显示区域，在此区域写点数据自动换行
入口参数：xy起点和终点
返回值：无
*************************************************/
//设置显示窗口
void Lcd_SetRegion(u8 xStar, u8 yStar,u8 xEnd,u8 yEnd) {
	LCD_WriteReg(0x38,xEnd);
	LCD_WriteReg(0x39,xStar);
	LCD_WriteReg(0x36,yEnd);
	LCD_WriteReg(0x37,yStar);
	LCD_WriteReg(0x21,xStar);
	LCD_WriteReg(0x20,yStar);
	Lcd_WriteIndex(0x22);	
}

	
/*************************************************
函数名：LCD_DrawPoint
功能：画一个点
入口参数：xy坐标和颜色数据
返回值：无
*************************************************/
void Gui_DrawPoint(u16 x,u16 y,u16 Data) {
	Lcd_SetXY(x,y);
	Lcd_WriteData_16Bit(Data);
}    

/*************************************************
函数名：Lcd_Clear
功能：全屏清屏函数
入口参数：填充颜色COLOR
返回值：无
*************************************************/
void Lcd_Clear(u16 Color) {	
   unsigned int i,m;
   Lcd_SetRegion(0,0,X_MAX_PIXEL-1,Y_MAX_PIXEL-1);
   for(i=0;i<X_MAX_PIXEL;i++) {
    for(m=0;m<Y_MAX_PIXEL;m++) {	
	  	Lcd_WriteData_16Bit(Color);
    }  		 
	 }
}

