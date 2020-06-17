#ifndef __SSD1306__
#define __SSD1306__

#include "stm32f4_fonts.h"
#include "I2C.h"

#define I2CX I2C3

#define SSD1306_I2C_ADDR         0x78
#define SSD1306_WIDTH            128
#define SSD1306_HEIGHT           64

static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
static uint16_t X = 0,Y = 0;

#define SSD1306_WRITECOMMAND(command)      I2C_Write(I2CX, SSD1306_I2C_ADDR, 0x00, (command))

extern void Init_I2C3();
extern void Init_SSD1306();
extern void SSD1306_Fill(int);
extern void SSD1306_UpdateScreen();
extern void SSD1306_LeftRolling();
extern void SSD1306_DrawPixel(uint16_t,uint16_t,int);
extern void SSD1306_GotoXY(uint16_t,uint16_t);
extern char SSD1306_Putc(char,FontDef_t*,int);
extern char SSD1306_Puts(char*,FontDef_t*,int);
extern void SSD1306_DrawLine(uint16_t,uint16_t,uint16_t,uint16_t,uint8_t);
extern void SSD1306_ON();
extern void SSD1306_ON();

#endif
