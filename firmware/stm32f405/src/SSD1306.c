#include "SSD1306.h"

void Init_I2C3(){

   GPIO_InitTypeDef GPIO_InitStructure;
   I2C_InitTypeDef I2C_InitStruct;

   RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C3, ENABLE);
   RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C3, ENABLE);
   RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C3, DISABLE);

   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);

   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
   GPIO_Init(GPIOC, &GPIO_InitStructure);

   GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_I2C3);
   GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_I2C3);

   I2C_InitStruct.I2C_ClockSpeed = 400000;
   I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
   I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
   I2C_InitStruct.I2C_OwnAddress1 = 0x00; 
   I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
   I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
   I2C_Init(I2C3, &I2C_InitStruct);

   I2C_Cmd(I2C3, ENABLE);

}

void Init_SSD1306()
{
   /* Init LCD */
   SSD1306_WRITECOMMAND(0xAE); //display off
   SSD1306_WRITECOMMAND(0x20); //Set Memory Addressing Mode   
   SSD1306_WRITECOMMAND(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
   SSD1306_WRITECOMMAND(0xB0); //Set Page Start Address for Page Addressing Mode,0-7
   SSD1306_WRITECOMMAND(0xC8); //Set COM Output Scan Direction
   SSD1306_WRITECOMMAND(0x00); //---set low column address
   SSD1306_WRITECOMMAND(0x10); //---set high column address
   SSD1306_WRITECOMMAND(0x40); //--set start line address
   SSD1306_WRITECOMMAND(0x81); //--set contrast control register
   SSD1306_WRITECOMMAND(0xFF);
   SSD1306_WRITECOMMAND(0xA1); //--set segment re-map 0 to 127
   SSD1306_WRITECOMMAND(0xA6); //--set normal display
   SSD1306_WRITECOMMAND(0xA8); //--set multiplex ratio(1 to 64)
   SSD1306_WRITECOMMAND(0x3F); //
   SSD1306_WRITECOMMAND(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
   SSD1306_WRITECOMMAND(0xD3); //-set display offset
   SSD1306_WRITECOMMAND(0x00); //-not offset
   SSD1306_WRITECOMMAND(0xD5); //--set display clock divide ratio/oscillator frequency
   SSD1306_WRITECOMMAND(0xF0); //--set divide ratio
   SSD1306_WRITECOMMAND(0xD9); //--set pre-charge period
   SSD1306_WRITECOMMAND(0x22); //
   SSD1306_WRITECOMMAND(0xDA); //--set com pins hardware configuration
   SSD1306_WRITECOMMAND(0x12);
   SSD1306_WRITECOMMAND(0xDB); //--set vcomh
   SSD1306_WRITECOMMAND(0x20); //0x20,0.77xVcc
   SSD1306_WRITECOMMAND(0x8D); //--set DC-DC enable
   SSD1306_WRITECOMMAND(0x14); //
   SSD1306_WRITECOMMAND(0xAF); //--turn on SSD1306 panel   
}

void SSD1306_Fill(int val)
{
   memset(SSD1306_Buffer,val, sizeof(SSD1306_Buffer));
}

void SSD1306_UpdateScreen()
{
   uint8_t m;
   for (m = 0; m < 8; m++) {
      SSD1306_WRITECOMMAND(0xB0 + m);
      SSD1306_WRITECOMMAND(0x00);
      SSD1306_WRITECOMMAND(0x10);
      /* Write multi data */
      I2C_WriteMulti(I2CX, SSD1306_I2C_ADDR, 0x40, &SSD1306_Buffer[SSD1306_WIDTH * m], SSD1306_WIDTH);
   }  
}

void SSD1306_LeftRolling()
{
   for(int x = 0;x < SSD1306_WIDTH-1;++x){
      for(int y = 0;y < SSD1306_HEIGHT;++y){
	 //SSD1306_Buffer[x + (y >> 3) * SSD1306_WIDTH] = SSD1306_Buffer[(x+1) + (y >> 3) * SSD1306_WIDTH];
	 SSD1306_Buffer[x + ((y >> 3) << 7)] = SSD1306_Buffer[(x+1) + ((y >> 3) << 7)];
      }
   }
   for(int y = 0;y < SSD1306_HEIGHT;++y){
      SSD1306_Buffer[127 + ((y >> 3) << 7)] = 0x00;
   }
}

void SSD1306_DrawPixel(uint16_t x, uint16_t y,int val) {
   if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT){
      /* Error */
      return;
   }

   /* Set color */
   if (val == 0xFF) {
      SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
   } else {
      SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
   }
}

void SSD1306_GotoXY(uint16_t x, uint16_t y) {
   /* Set write pointers */
   X = x;
   Y = y;
}

char SSD1306_Putc(char ch, FontDef_t* Font,int val){
   uint32_t i, b, j;
   /* Check available space in LCD */
   if (SSD1306_WIDTH <= (X + Font->FontWidth) || SSD1306_HEIGHT <= (Y + Font->FontHeight)){
      /* Error */
      return 0;
   }

   /* Go through font */
   for (i = 0; i < Font->FontHeight; i++) {
      b = Font->data[(ch - 32) * Font->FontHeight + i];
      for (j = 0; j < Font->FontWidth; j++) {
	 if ((b << j) & 0x8000) {
	    SSD1306_DrawPixel(X + j, (Y + i), val);
	 } else {
	    SSD1306_DrawPixel(X + j, (Y + i), !val);
	 }
      }
   }   

   /* Increase pointer */
   X += Font->FontWidth;

   /* Return character written */
   return ch;
}

char SSD1306_Puts(char* str, FontDef_t* Font,int val) {
   /* Write characters */
   while (*str) {
      /* Write character by character */
      if (SSD1306_Putc(*str, Font, val) != *str) {
	 /* Return error */
	 return *str;
      }

      /* Increase string pointer */
      str++;
   }

   /* Everything OK, zero should be returned */
   return *str;
}

void SSD1306_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t c) {
   int16_t dx, dy, sx, sy, err, e2, i, tmp; 

   /* Check for overflow */
   if (x0 >= SSD1306_WIDTH) {
      x0 = SSD1306_WIDTH - 1;
   }
   if (x1 >= SSD1306_WIDTH) {
      x1 = SSD1306_WIDTH - 1;
   }
   if (y0 >= SSD1306_HEIGHT) {
      y0 = SSD1306_HEIGHT - 1;
   }
   if (y1 >= SSD1306_HEIGHT) {
      y1 = SSD1306_HEIGHT - 1;
   }

   dx = (x0 < x1) ? (x1 - x0) : (x0 - x1); 
   dy = (y0 < y1) ? (y1 - y0) : (y0 - y1); 
   sx = (x0 < x1) ? 1 : -1; 
   sy = (y0 < y1) ? 1 : -1; 
   err = ((dx > dy) ? dx : -dy) / 2; 

   if (dx == 0) {
      if (y1 < y0) {
	 tmp = y1;
	 y1 = y0;
	 y0 = tmp;
      }

      if (x1 < x0) {
	 tmp = x1;
	 x1 = x0;
	 x0 = tmp;
      }

      /* Vertical line */
      for (i = y0; i <= y1; i++) {
	 SSD1306_DrawPixel(x0, i, c);
      }

      /* Return from function */
      return;
   }

   if (dy == 0) {
      if (y1 < y0) {
	 tmp = y1;
	 y1 = y0;
	 y0 = tmp;
      }

      if (x1 < x0) {
	 tmp = x1;
	 x1 = x0;
	 x0 = tmp;
      }

      /* Horizontal line */
      for (i = x0; i <= x1; i++) {
	 SSD1306_DrawPixel(i, y0, c);
      }

      /* Return from function */
      return;
   }

   while (1) {
      SSD1306_DrawPixel(x0, y0, c);
      if (x0 == x1 && y0 == y1) {
	 break;
      }
      e2 = err; 
      if (e2 > -dx) {
	 err -= dy;
	 x0 += sx;
      } 
      if (e2 < dy) {
	 err += dx;
	 y0 += sy;
      } 
   }
}

void SSD1306_ON(void) {
   SSD1306_WRITECOMMAND(0x8D);  
   SSD1306_WRITECOMMAND(0x14);  
   SSD1306_WRITECOMMAND(0xAF);  
}

void SSD1306_OFF(void) {
   SSD1306_WRITECOMMAND(0x8D);  
   SSD1306_WRITECOMMAND(0x10);
   SSD1306_WRITECOMMAND(0xAE);  
}


