#ifndef _LCD_DRIVER_H_
#define _LCD_DRIVER_H_

#include "stm32f10x.h"

//-------------------------屏幕物理像素设置--------------------------------------//
#define LCD_X_SIZE	        176
#define LCD_Y_SIZE	        220

#define X_MAX_PIXEL	        LCD_Y_SIZE
#define Y_MAX_PIXEL	        LCD_X_SIZE
//////////////////////////////////////////////////////////////////////
	 
#define RED  	0xf800
#define GREEN	0x07e0
#define WHITE	0xffff
#define BLACK			0x0000
#define BLUE 			0x001f

#define NAVY_BLUE		0x0010		//海军蓝
#define DARK_BLUE		0x0011		//暗蓝
#define MED_BLUE		0x0019		//中蓝
#define INT_BLUE		0x0174		//国际蓝
#define PRU_BLUE		0x018A		//普鲁士蓝
#define DARK_P_BLUE		0x0193		//暗婴儿蓝
#define ULTRAMARINE		0x019F		//海蓝
#define MAARINE_BLUE	0x022F		//水手蓝
#define COBALT_BLUE		0x0235		//钴蓝
#define MINERAL_BLUE	0x0273		//矿蓝
#define STRONG_BLUE		0x030E		//浓蓝
#define DARK_GREEN		0x0320		//暗绿
#define AZURE			0x03FF		//湛蓝
#define GREEN2			0x0400		//纯绿
#define TEAL			0x0410		//水鸭色


#define NAVY			0x000F 					//深蓝色 ?0, ? 0, 127
#define DGREEN		0x03E0 					//深绿色 ?0, ?127, ?0
#define DCYAN			0x03EF					//深青色 ?0, ?127, 127 ? ? ??
#define MAROON		0x7800 					//深红色 ?127, ? 0, ? 0 ? ? ?
#define PURPLE		0x780F 					//紫色 ? ?127, ? 0, 127 ? ? ?
#define OLIVE			0x7BE0 					//橄榄绿 ?127, 127, ? 0 ? ? ?
#define LGRAY			0xC618 					//灰白色 ?192, 192, 192 ? ? ?
#define DGRAY			0x7BEF 					//深灰色 ?127, 127, 127 ? ? ?
#define GREEN1			0x07E0 					//绿色 ? ?0, 255, ? 0 ? ? ? ?
#define CYAN			0x07FF 					//青色 ? ?0, 255, 255 ? ? ? ?
#define MAGENTA		0xF81F 					//品红 ? ?255, ? 0, 255 ? ? ?
#define YELLOW 		0xFFE0 					//黄色 ? ?255, 255, 0 ? ? ? ?
#define GRAY0   0xEF7D   	//灰色0 3165 00110 001011 00101
#define GRAY1   0x8410      	//灰色1      00000 000000 00000
#define GRAY2   0x4208      	//灰色2  1111111111011111

#if 0
#define LCD_LED 					GPIO_Pin_1
#define LCD_LED_PORT              	GPIOB
#define LCD_LED_CLK               	RCC_APB2Periph_GPIOB
#define	LCD_LED_SET				LCD_LED_PORT->BSRR = LCD_LED
#define	LCD_LED_CLR				LCD_LED_PORT->BRR = LCD_LED

#define LCD_RS 						GPIO_Pin_7
#define LCD_RS_PORT              	GPIOA
#define LCD_RS_CLK               	RCC_APB2Periph_GPIOA
#define	LCD_RS_SET				LCD_RS_PORT->BSRR = LCD_RS
#define	LCD_RS_CLR				LCD_RS_PORT->BRR = LCD_RS

#define LCD_CS 						GPIO_Pin_4
#define LCD_CS_PORT              	GPIOA
#define LCD_CS_CLK               	RCC_APB2Periph_GPIOA
#define	LCD_CS_SET				LCD_CS_PORT->BSRR = LCD_CS
#define	LCD_CS_CLR				LCD_CS_PORT->BRR = LCD_CS

#define LCD_SCL 					GPIO_Pin_5
#define LCD_SCL_PORT              	GPIOA
#define LCD_SCL_CLK               	RCC_APB2Periph_GPIOA
#define	LCD_SCL_SET				LCD_CS_PORT->BSRR = LCD_SCL
#define	LCD_SCL_CLR				LCD_CS_PORT->BRR = LCD_SCL

#define LCD_SDA 					GPIO_Pin_6
#define LCD_SDA_PORT              	GPIOA
#define LCD_SDA_CLK               	RCC_APB2Periph_GPIOA
#define	LCD_SDA_SET				LCD_CS_PORT->BSRR = LCD_SDA
#define	LCD_SDA_CLR				LCD_CS_PORT->BRR = LCD_SDA

#define LCD_RST						GPIO_Pin_0
#define LCD_RST_PORT              	GPIOB
#define LCD_RST_CLK               	RCC_APB2Periph_GPIOB
#define	LCD_RST_SET				LCD_LED_PORT->BSRR = LCD_RST
#define	LCD_RST_CLR				LCD_LED_PORT->BRR = LCD_RST

#else

#define LCD_CS      GPIO_Pin_1
#define LCD_CS_PORT               GPIOB
#define LCD_CS_CLK                RCC_APB2Periph_GPIOB
#define LCD_CS_SET    LCD_CS_PORT->BSRR = LCD_CS
#define LCD_CS_CLR    LCD_CS_PORT->BRR = LCD_CS

#define LCD_RS       GPIO_Pin_7
#define LCD_RS_PORT               GPIOA
#define LCD_RS_CLK                RCC_APB2Periph_GPIOA
#define LCD_RS_SET    LCD_RS_PORT->BSRR = LCD_RS
#define LCD_RS_CLR    LCD_RS_PORT->BRR = LCD_RS

#define LCD_LED       GPIO_Pin_4
#define LCD_LED_PORT               GPIOA
#define LCD_LED_CLK                RCC_APB2Periph_GPIOA
#define LCD_LED_SET    LCD_LED_PORT->BSRR = LCD_LED
#define LCD_LED_CLR    LCD_LED_PORT->BRR = LCD_LED

#define LCD_SCL      GPIO_Pin_5
#define LCD_SCL_PORT               GPIOA
#define LCD_SCL_CLK                RCC_APB2Periph_GPIOA
#define LCD_SCL_SET    LCD_SCL_PORT->BSRR = LCD_SCL
#define LCD_SCL_CLR    LCD_SCL_PORT->BRR = LCD_SCL

#define LCD_SDA      GPIO_Pin_6
#define LCD_SDA_PORT               GPIOA
#define LCD_SDA_CLK                RCC_APB2Periph_GPIOA
#define LCD_SDA_SET    LCD_SDA_PORT->BSRR = LCD_SDA
#define LCD_SDA_CLR    LCD_SDA_PORT->BRR = LCD_SDA

#define LCD_RST      GPIO_Pin_0
#define LCD_RST_PORT               GPIOB
#define LCD_RST_CLK                RCC_APB2Periph_GPIOB
#define LCD_RST_SET    LCD_RST_PORT->BSRR = LCD_RST
#define LCD_RST_CLR    LCD_RST_PORT->BRR = LCD_RST
#endif



void LCD_GPIO_Init(void);
void Lcd_WriteIndex(u8 Index);
void Lcd_WriteData(u8 Data);
void Lcd_WriteReg(u8 Index,u8 Data);
u16 Lcd_ReadReg(u8 LCD_Reg);
void Lcd_Reset(void);
void Lcd_Init(void);
void Lcd_Clear(u16 Color);
void Lcd_SetXY(u16 x,u16 y);
void Gui_DrawPoint(u16 x,u16 y,u16 Data);
unsigned int Lcd_ReadPoint(u16 x,u16 y);
void Lcd_SetRegion(u8 x_start,u8 y_start,u8 x_end,u8 y_end);
void Lcd_WriteData_16Bit(u16 Data);

#endif
