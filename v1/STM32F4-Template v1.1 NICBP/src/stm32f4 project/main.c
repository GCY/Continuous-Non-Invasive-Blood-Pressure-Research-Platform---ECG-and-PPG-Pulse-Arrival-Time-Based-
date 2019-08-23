#include <stdio.h>
#include <math.h>
#include <stm32f4xx.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_usart.h>
#include <stm32f4xx_exti.h>
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_adc.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_dma.h>
#include <stm32f4xx_rtc.h>

#include "tiny_printf.h"
#include "stm32f4_fonts.h"

volatile uint32_t TimingDelay;

void Delay(__IO uint32_t nTime)
{
   TimingDelay = nTime;
   while(TimingDelay){
   }
}

void SysTick_Handler(void)
{
   if(TimingDelay){
      --TimingDelay;
   }
}

void init_I2C2(){

    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_I2C2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_I2C2);

    // 設定 I2C1 
    I2C_InitStruct.I2C_ClockSpeed = 100000; // 設定 I2C 時鐘速度為 100kHz
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C; // I2C 模式
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2; // 50% duty cycle --> standard
    I2C_InitStruct.I2C_OwnAddress1 = 0x00; // own address, not relevant in master mode
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable; // disable acknowledge when reading (can be changed later on)
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // 設定 I2C 地址長度為 7 bit
    I2C_Init(I2C2, &I2C_InitStruct); // 初始化 I2C1

    // 啟用 I2C1
    I2C_Cmd(I2C2, ENABLE);

}


void init_I2C3(){

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

    // 設定 I2C1 
    I2C_InitStruct.I2C_ClockSpeed = 100000; // 設定 I2C 時鐘速度為 100kHz
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C; // I2C 模式
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2; // 50% duty cycle --> standard
    I2C_InitStruct.I2C_OwnAddress1 = 0x00; // own address, not relevant in master mode
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable; // disable acknowledge when reading (can be changed later on)
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // 設定 I2C 地址長度為 7 bit
    I2C_Init(I2C3, &I2C_InitStruct); // 初始化 I2C1

    // 啟用 I2C1
    I2C_Cmd(I2C3, ENABLE);

}

void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction){
    // wait until I2C1 is not busy anymore
    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

    // Send I2C1 START condition 
    I2C_GenerateSTART(I2Cx, ENABLE);

    // wait for I2C1 EV5 --> Slave has acknowledged start condition
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

    // Send slave Address for write 
    I2C_Send7bitAddress(I2Cx, address, direction);

    /* wait for I2C1 EV6, check if 
    * either Slave has acknowledged Master transmitter or
    * Master receiver mode, depending on the transmission
    * direction
    */ 
    if(direction == I2C_Direction_Transmitter){
        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    } else if(direction == I2C_Direction_Receiver){
        while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    }
}

/* This function transmits one byte to the slave device
* Parameters:
* I2Cx --> the I2C peripheral e.g. I2C1 
* data --> the data byte to be transmitted
*/
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data)
{
    I2C_SendData(I2Cx, data);
    // wait for I2C1 EV8_2 --> byte has been transmitted
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

/* This function reads one byte from the slave device 
* and acknowledges the byte (requests another byte)
*/
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx){
    uint8_t data;
    // enable acknowledge of recieved data
    I2C_AcknowledgeConfig(I2Cx, ENABLE);
    // wait until one byte has been received
    while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
    // read data from I2C data register and return data byte
    data = I2C_ReceiveData(I2Cx);
    return data;
}

/* This function reads one byte from the slave device
* and doesn't acknowledge the recieved data 
*/
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx){
    uint8_t data;
    // disabe acknowledge of received data
    // nack also generates stop condition after last byte received
    // see reference manual for more info
    I2C_AcknowledgeConfig(I2Cx, DISABLE);
    I2C_GenerateSTOP(I2Cx, ENABLE);
    // wait until one byte has been received
    while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
    // read data from I2C data register and return data byte
    data = I2C_ReceiveData(I2Cx);
    return data;
}

/* This funtion issues a stop condition and therefore
* releases the bus
*/
void I2C_stop(I2C_TypeDef* I2Cx){
    // Send I2C1 STOP Condition 
    I2C_GenerateSTOP(I2Cx, ENABLE);
}

void I2C_Write(I2C_TypeDef* I2Cx, uint8_t address, uint8_t reg, uint8_t data) {
	I2C_start(I2Cx, address, I2C_Direction_Transmitter);
	I2C_write(I2Cx, reg);
	I2C_write(I2Cx, data);
	I2C_stop(I2Cx);
}

void I2C_WriteMulti(I2C_TypeDef* I2Cx, uint8_t address, uint8_t reg, uint8_t* data, uint16_t count) {
	I2C_start(I2Cx, address, I2C_Direction_Transmitter);
	I2C_write(I2Cx, reg);
	while (count--) {
		I2C_write(I2Cx, *data++);
	}
	I2C_stop(I2Cx);
}

volatile uint32_t adc_value[2] = {0};
void Init_ADC(void)
{
   GPIO_InitTypeDef gpio;
   DMA_InitTypeDef dma;
   NVIC_InitTypeDef nvic;
   ADC_InitTypeDef adc;
   ADC_CommonInitTypeDef adc_com;

   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC,ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3,ENABLE);

   dma.DMA_Channel = DMA_Channel_2;
   dma.DMA_PeripheralBaseAddr = (uint32_t)0x4001224C;
   dma.DMA_Memory0BaseAddr = (uint32_t)&adc_value;
   dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
   dma.DMA_BufferSize = 2;
   dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
   dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
   dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
   dma.DMA_Mode = DMA_Mode_Circular;
   dma.DMA_Priority = DMA_Priority_High;
   dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
   dma.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
   dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
   dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
   DMA_Init(DMA2_Stream0, &dma);
   DMA_Cmd(DMA2_Stream0, ENABLE);

   gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
   gpio.GPIO_Mode = GPIO_Mode_AN;
   gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOC,&gpio);

   adc_com.ADC_Mode = ADC_Mode_Independent;
   adc_com.ADC_Prescaler = ADC_Prescaler_Div2;
   adc_com.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
   adc_com.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
   ADC_CommonInit(&adc_com);

   adc.ADC_Resolution = ADC_Resolution_12b;
   adc.ADC_ScanConvMode = ENABLE;
   adc.ADC_ContinuousConvMode = ENABLE;
   adc.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
   adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
   adc.ADC_DataAlign = ADC_DataAlign_Right;
   adc.ADC_NbrOfConversion = 2;
   ADC_Init(ADC3,&adc);

   ADC_RegularChannelConfig(ADC3,ADC_Channel_10,1,ADC_SampleTime_15Cycles);
   ADC_RegularChannelConfig(ADC3,ADC_Channel_11,2,ADC_SampleTime_15Cycles);
   ADC_DMARequestAfterLastTransferCmd(ADC3,ENABLE);
   ADC_DMACmd(ADC3,ENABLE);
   ADC_Cmd(ADC3,ENABLE);

   ADC_SoftwareStartConv(ADC3);
   
}

void USART2_IRQHandler(void)
{
   if(USART_GetITStatus(USART2,USART_IT_RXNE) != RESET){
      static uint32_t index = 0;
      char c = USART2->DR;

      USART_SendData(USART2,c);
   }
}

void Init_USART(void){
   USART_InitTypeDef usart;
   GPIO_InitTypeDef usart_pin;
   NVIC_InitTypeDef interrup;

   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

   GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);
   GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);

   usart_pin.GPIO_OType = GPIO_OType_PP;
   usart_pin.GPIO_PuPd = GPIO_PuPd_UP;
   usart_pin.GPIO_Mode = GPIO_Mode_AF;
   usart_pin.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_2;
   usart_pin.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOA,&usart_pin);

   usart.USART_BaudRate = 115200;
   usart.USART_WordLength = USART_WordLength_8b;
   usart.USART_StopBits = USART_StopBits_1;
   usart.USART_Parity = USART_Parity_No;
   usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
   USART_Init(USART2,&usart);

   USART_Cmd(USART2,ENABLE);

   USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);

   interrup.NVIC_IRQChannel = USART2_IRQn;
   interrup.NVIC_IRQChannelPreemptionPriority = 0;
   interrup.NVIC_IRQChannelSubPriority = 0;
   interrup.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&interrup);

   NVIC_EnableIRQ(USART2_IRQn);
}

void USART_Send_String(volatile char *string)
{
   while(*string){
      USART_SendData(USART2,*string);
      ++string;
      while(USART_GetFlagStatus(USART2,USART_FLAG_TC) == RESET){}
   }
}

#define SSD1306_I2C_ADDR         0x78
#define SSD1306_WIDTH            128
#define SSD1306_HEIGHT           64
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
uint16_t X = 0,Y = 0;
#define SSD1306_WRITECOMMAND(command)      I2C_Write(I2C3, SSD1306_I2C_ADDR, 0x00, (command))
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
      I2C_WriteMulti(I2C3, SSD1306_I2C_ADDR, 0x40, &SSD1306_Buffer[SSD1306_WIDTH * m], SSD1306_WIDTH);
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


double prev_w = 0.0f;
uint16_t DCRemoval(uint16_t x)
{
   double w_n = (double)x + (0.9f * prev_w);
   double result = w_n - prev_w;
   prev_w = w_n;
   return (uint16_t)result;
}

int main(void)
{
   if(SysTick_Config(SystemCoreClock / 1000 / 1000)){
      while(1){}
   }

  Init_USART();
  Init_ADC();

  init_I2C2();
  init_I2C3();
  Init_SSD1306();

#define MAX30100_ADDRESS 0xAE
#define MAX30100_INT_STATUS 0x00
#define MAX30100_INT_ENABLE     0x01
#define MAX30100_FIFO_WR_PTR    0x02 
#define MAX30100_OVRFLOW_CTR    0x03 
#define MAX30100_FIFO_RD_PTR    0x04  
#define MAX30100_FIFO_DATA      0x05 
#define MAX30100_MODE_CONFIG    0x06 
#define MAX30100_SPO2_CONFIG    0x07 
#define MAX30100_LED_CONFIG     0x09 
#define MAX30100_TEMP_INTG      0x16 
#define MAX30100_TEMP_FRAC      0x17  
#define MAX30100_REV_ID         0xFE  
#define MAX30100_PART_ID        0xFF 
#define POR_PART_ID             0x11
typedef enum{
   MAX30100_MODE_HR_ONLY = 0x02,
   MAX30100_MODE_SPO2_HR = 0x03
}Mode;
typedef enum{ // This is the same for both LEDs
  pw200,    // 200us pulse
  pw400,    // 400us pulse
  pw800,    // 800us pulse
  pw1600    // 1600us pulse
}pulseWidth;

typedef enum{
  sr50,    // 50 samples per second
  sr100,   // 100 samples per second
  sr167,   // 167 samples per second
  sr200,   // 200 samples per second
  sr400,   // 400 samples per second
  sr600,   // 600 samples per second
  sr800,   // 800 samples per second
  sr1000   // 1000 samples per second
}sampleRate;
typedef enum{
  so2,	// SO2 interrupt
  hr,	// Heart-rate interrupt
  temp,	// Temperature interrupt
  full,	// FIFO full interrupt
}interruptSource;
typedef enum{
  i0,    // No current
  i4,    // 4.4mA
  i8,    // 7.6mA
  i11,   // 11.0mA
  i14,   // 14.2mA
  i17,   // 17.4mA
  i21,   // 20.8mA
  i24,	 // 24mA
  i27,   // 27.1mA
  i31,   // 30.6mA
  i34,   // 33.8mA
  i37,   // 37.0mA
  i40,   // 40.2mA
  i44,   // 43.6mA
  i47,   // 46.8mA
  i50    // 50.0mA
}ledCurrent;
  I2C_Write(I2C2,MAX30100_ADDRESS,MAX30100_MODE_CONFIG,MAX30100_MODE_SPO2_HR);
  I2C_Write(I2C2,MAX30100_ADDRESS,MAX30100_LED_CONFIG,(i4 << 4) | i4);
  I2C_Write(I2C2,MAX30100_ADDRESS,MAX30100_SPO2_CONFIG,(sr100<<2)|pw1600);
  Delay(5000);
  I2C_start(I2C2, MAX30100_ADDRESS, I2C_Direction_Transmitter); 
  I2C_write(I2C2, MAX30100_FIFO_DATA); 
  I2C_stop(I2C2); 
  uint8_t received_data[4];
  I2C_start(I2C2, MAX30100_ADDRESS, I2C_Direction_Receiver);
  received_data[0] = I2C_read_ack(I2C2); 
  received_data[1] = I2C_read_ack(I2C2);
  received_data[2] = I2C_read_ack(I2C2);
  received_data[3] = I2C_read_nack(I2C2); 
  I2C_stop(I2C2);
  int16_t data = received_data[0] << 8 | received_data[1];
  uint16_t IR = (received_data[0]<<8) | received_data[1];;      // Last IR reflectance datapoint
  uint16_t RED = (received_data[2]<<8) | received_data[3];;     // Last Red reflectance datapoint
  

  SSD1306_Fill(0x00);
  SSD1306_UpdateScreen();
  Delay(100000);
  SSD1306_Fill(0xFF);
  SSD1306_UpdateScreen();
  Delay(100000);
  SSD1306_Fill(0x00);
  SSD1306_UpdateScreen();
  Delay(100000);

  SSD1306_GotoXY(3, 4);
  SSD1306_Puts("NICBP", &Font_11x18, 0xFF);
  SSD1306_GotoXY(3, 25);
  SSD1306_Puts("20180212", &Font_11x18, 0xFF);
  SSD1306_GotoXY(3, 45);
  SSD1306_Puts("TonyGUO", &Font_11x18, 0xFF);
  SSD1306_UpdateScreen();  
  Delay(100000);
  SSD1306_Fill(0x00);
  SSD1306_UpdateScreen();
  Delay(100000);


  char str[256] = "";

  USART_Send_String(str);

  uint16_t idx = 0;
  uint16_t lx = 0,ly = 63;
  uint16_t IR_L = 0;
  uint32_t avg_L = 0;
  while(1){

     uint32_t avg = 0;
     for(int i = 0;i < 1000;++i){
	avg += adc_value[0];
     }

     avg /= 1000;
/*
     avg *= 3.3;
     avg /= 4096;*/
/*
     str[0] = ((avg/1000)%10) + 48;
     str[1] = '.';
     str[2] = ((avg/100)%10) + 48;
     str[3] = (avg%10) + 48;
     str[4] = ' ';
*/

     /*Left shift mode*/
    if(idx >= 128){
	SSD1306_LeftRolling();
	SSD1306_DrawLine(126,ly,127,64-(((avg*10000/33*4096)/64000000)),0xFF);
	ly = 64-(((avg*10000/33*4096)/64000000));
	SSD1306_UpdateScreen();
     }
     else{
	SSD1306_DrawLine(lx,ly,idx,64-(((avg*10000/33*4096)/64000000)),0xFF);
	lx = idx;ly = 64-(((avg*10000/33*4096)/64000000));
	SSD1306_UpdateScreen();
	++idx;
     }
     
      
  I2C_start(I2C2, MAX30100_ADDRESS, I2C_Direction_Transmitter); 
  I2C_write(I2C2, MAX30100_FIFO_DATA); 
  I2C_stop(I2C2); 
  uint8_t received_data[4];
  I2C_start(I2C2, MAX30100_ADDRESS, I2C_Direction_Receiver);
  received_data[0] = I2C_read_ack(I2C2); 
  received_data[1] = I2C_read_ack(I2C2);
  received_data[2] = I2C_read_ack(I2C2);
  received_data[3] = I2C_read_nack(I2C2); 
  I2C_stop(I2C2);
  uint16_t IR = (received_data[0]<<8) | received_data[1];;      // Last IR reflectance datapoint
  uint16_t RED = (received_data[2]<<8) | received_data[3];;     // Last Red reflectance datapoint

  /*
  str[5] = ',';
  str[6] = ((IR/10000)%10)+48;
  str[7] = ((IR/1000)%10)+48;
  str[8] = ((IR/100)%10)+48;;
  str[9] = ((IR/10)%10)+48;;
  str[10] = ((IR/1)%10)+48;;
  str[11] = ',';
  str[12] = ((RED/10000)%10)+48;
  str[13] = ((RED/1000)%10)+48;
  str[14] = ((RED/100)%10)+48;;
  str[15] = ((RED/10)%10)+48;;
  str[16] = ((RED/1)%10)+48;;     
  str[17] = '\n';
  str[18] = 0;
  */
  sprintf(str,"%d,%d,%d\n",avg,IR,RED);
  USART_Send_String(str);
  }
 
  return(0);
}    
