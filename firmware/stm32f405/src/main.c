#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include <stm32f4xx.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_usart.h>
#include <stm32f4xx_exti.h>
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_adc.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_dma.h>
#include <stm32f4xx_rtc.h>
#include <stm32f4xx_tim.h>

#include "adc.h"
#include "VCP.h"
#include "I2C.h"
#include "MAX3010x.h"
#include "SSD1306.h"

#include "tiny_printf.h"
#include "stm32f4_fonts.h"

#define __FPU_PRESENT
#define __FPU_USED

volatile uint32_t TimingDelay;

volatile uint32_t micros = 0;

enum{
   SPO2_SIGNAL_MODE = 1,
   SPO2_VALUE_MODE,
   ECG_SIGNAL_MODE
};
volatile bool clear_flag = false;
volatile uint32_t mode = SPO2_SIGNAL_MODE;

volatile uint32_t max_set_time = 0;
volatile int32_t max_value = 100;
volatile int32_t current_max = 100;

int64_t map(int64_t x, int64_t in_min, int64_t in_max, int64_t out_min, int64_t out_max) {
   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

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
   ++micros;
}

void Init_LED()
{
   //Enable the GPIOD Clock
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);


   // GPIOD Configuration
   GPIO_InitTypeDef GPIO_InitStruct;
   GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
   GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;

   //GPIO_SetBits(GPIOC, GPIO_Pin_12);
   //GPIO_SetBits(GPIOC, GPIO_Pin_13);
   GPIO_Init(GPIOC, &GPIO_InitStruct);   
}

//volatile uint32_t adc_value[4] = {0};
/*void Init_ADC(void)
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
   dma.DMA_BufferSize = 4;
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

   gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
   gpio.GPIO_Mode = GPIO_Mode_AN;
   gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOC,&gpio);

   adc_com.ADC_Mode = ADC_Mode_Independent;
   adc_com.ADC_Prescaler = ADC_Prescaler_Div4;
   adc_com.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
   adc_com.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
   ADC_CommonInit(&adc_com);

   adc.ADC_Resolution = ADC_Resolution_12b;
   adc.ADC_ScanConvMode = ENABLE;
   adc.ADC_ContinuousConvMode = ENABLE;
   adc.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
   adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
   adc.ADC_DataAlign = ADC_DataAlign_Right;
   adc.ADC_NbrOfConversion = 4;
   ADC_Init(ADC3,&adc);

   ADC_RegularChannelConfig(ADC3,ADC_Channel_10,1,ADC_SampleTime_3Cycles);
   ADC_RegularChannelConfig(ADC3,ADC_Channel_11,2,ADC_SampleTime_3Cycles);
   ADC_RegularChannelConfig(ADC3,ADC_Channel_12,3,ADC_SampleTime_3Cycles);
   ADC_RegularChannelConfig(ADC3,ADC_Channel_13,4,ADC_SampleTime_3Cycles);
   ADC_DMARequestAfterLastTransferCmd(ADC3,ENABLE);
   ADC_Cmd(ADC3,ENABLE);
   ADC_SoftwareStartConv(ADC3);
   ADC_DMACmd(ADC3,ENABLE);


}*/

void EXTILine0_Config(void)
{
   EXTI_InitTypeDef   EXTI_InitStructure;
   GPIO_InitTypeDef   GPIO_InitStructure;
   NVIC_InitTypeDef   NVIC_InitStructure;

   /* Enable GPIOB clock */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   /* Enable SYSCFG clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

   /* Configure PB0 pin as input floating */
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   /* Connect EXTI Line0 to PB0 pin */
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

   /* Configure EXTI Line0 */
   EXTI_InitStructure.EXTI_Line = EXTI_Line0;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);

   /* Enable and set EXTI Line0 Interrupt to the lowest priority */
   NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
}

int main(void)
{
   if(SysTick_Config(SystemCoreClock / 1000 / 1000)){
      while(1){}
   }
   
   EXTILine0_Config();
   Delay(5000);

   USBVCPInit();
   Delay(5000);

   Init_MAX3010x();
   Delay(5000);
   Init_I2C3();
   Delay(5000);
   Init_SSD1306();
   Delay(5000);

   //Init_ADC();
   ADC_4_Channel_Init();
   Init_LED();

   __IO uint32_t HSEStatus = 0;
   HSEStatus = RCC->CR & RCC_CR_HSERDY;
   if(HSEStatus){
      GPIO_SetBits(GPIOC, GPIO_Pin_12);
   }
   else{
      GPIO_ResetBits(GPIOC, GPIO_Pin_12);
   }   

   SSD1306_Fill(0x00);
   SSD1306_UpdateScreen();
   Delay(1000000);
   SSD1306_Fill(0xFF);
   SSD1306_UpdateScreen();
   Delay(1000000);
   SSD1306_Fill(0x00);
   SSD1306_UpdateScreen();  
   Delay(1000000);   

   SSD1306_GotoXY(3, 4);
   SSD1306_Puts("NICBP v2", &Font_11x18, 0xFF);
   SSD1306_GotoXY(3, 25);
   SSD1306_Puts("20190817", &Font_11x18, 0xFF);
   SSD1306_GotoXY(3, 45);
   SSD1306_Puts("TonyGUO", &Font_11x18, 0xFF);
   SSD1306_UpdateScreen();  
   Delay(1000000);
   SSD1306_Fill(0x00);
   SSD1306_UpdateScreen();

   Filter_Data irf = {0};
   Filter_Data redf = {0};  

   uint32_t irACValueSqSum = 0;
   uint32_t redACValueSqSum = 0;
   uint16_t samplesRecorded = 0;
   uint16_t pulsesDetected = 0;
   uint32_t currentSaO2Value = 0;    

   int id_state = 0;
   int recheck_state = 0;
   int sn = 1;  

   uint16_t idx = 0;
   uint16_t lx = 0,ly = 63;

   bool blue_flag = false;
   while(1){
      if(blue_flag){
	 GPIO_SetBits(GPIOC, GPIO_Pin_13);
      }
      else{
	 GPIO_ResetBits(GPIOC, GPIO_Pin_13);
      }  

      Read_MAX3010x();

      int32_t ORG_IR = IR;
      int32_t ORG_RED = RED;

      int32_t IRac = DCRemove(IR,&IRcw);
      int32_t REDac = DCRemove(RED,&REDcw);

      IRac = MeanDiff(IRac);
      REDac = MeanDiff(REDac);

      IRac = LowPassButterworthFilter(IRac,&irf);
      REDac = LowPassButterworthFilter(REDac,&redf);


      irACValueSqSum += IRac * IRac;
      redACValueSqSum += REDac * REDac;
      samplesRecorded++;  
      if(detectPulse(IRac)){
	 pulsesDetected++;


	 float red_log_rms = log( sqrt(redACValueSqSum/samplesRecorded) );
	 float ir_log_rms = log( sqrt(irACValueSqSum/samplesRecorded) );
	 float ratioRMS = 0.0f;
	 if(red_log_rms != 0.0f && ir_log_rms != 0.0f){
	    ratioRMS = red_log_rms / ir_log_rms;
	 }
	 currentSaO2Value = 110.0f - 14.0f * ratioRMS;	// SPo2 value by pulse-rate


	 if(pulsesDetected % RESET_SPO2_EVERY_N_PULSES == 0){
	    irACValueSqSum = 0;
	    redACValueSqSum = 0;
	    samplesRecorded = 0;
	 }     
      }


      BalanceIntesities();
     

      if(mode == SPO2_SIGNAL_MODE){
	 if( (micros - max_set_time) > 1500000){
	    max_set_time = micros;
	    max_value = current_max;
	    current_max = 100;
	 }
	 else{
	    if(abs(REDac) > current_max){
	       current_max = REDac;
	    }
	 }	 
	 if(clear_flag){
	    SSD1306_Fill(0x00);
	    SSD1306_UpdateScreen();
	    clear_flag = false;
	    
	    idx = 0;
	    lx = 0,ly = 63;	    
	 }
	 if(idx >= 128){
	    SSD1306_LeftRolling();
	    SSD1306_DrawLine(126,ly,127,map(REDac,-max_value,max_value,0,63),0xFF);
	    ly = map(REDac,-max_value,max_value,0,63);
	    SSD1306_UpdateScreen();
	 }
	 else{
	    SSD1306_DrawLine(lx,ly,idx,map(REDac,-max_value,max_value,0,63),0xFF);
	    lx = idx;ly = map(REDac,-max_value,max_value,0,63);
	    SSD1306_UpdateScreen();
	    ++idx;
	 }      
      }
      else if(mode == SPO2_VALUE_MODE){   
	 if(clear_flag){
	    SSD1306_Fill(0x00);
	    SSD1306_UpdateScreen();
	    clear_flag = false;
	 }	 
	 
	 unsigned char spo2_str[255];
	 sprintf(spo2_str,"SpO2 : %d",currentSaO2Value);
	 SSD1306_GotoXY(3, 25);
	 SSD1306_Puts(spo2_str, &Font_11x18, 0xFF);
	 SSD1306_UpdateScreen();
 	 
      }
      else if(mode == ECG_SIGNAL_MODE && ADC3_ready){
	 if( (micros - max_set_time) > 1500000){
	    max_set_time = micros;
	    max_value = current_max;
	    current_max = 100;
	 }
	 else{
	    if(ADC3_value[0] > current_max){
	       current_max = ADC3_value[0];
	    }
	 }		 
	 if(clear_flag){
	    SSD1306_Fill(0x00);
	    SSD1306_UpdateScreen();
	    clear_flag = false;
	    
	    idx = 0;
	    lx = 0,ly = 63;	    
	 }	 
	 if(idx >= 128){
	    SSD1306_LeftRolling();
	    SSD1306_DrawLine(126,ly,127,map(ADC3_value[0],-max_value,max_value,0,63),0xFF);
	    ly = map(ADC3_value[0],-max_value,max_value,0,63);
	    SSD1306_UpdateScreen();
	 }
	 else{
	    SSD1306_DrawLine(lx,ly,idx,map(ADC3_value[0],-max_value,max_value,0,63),0xFF);
	    lx = idx;ly = map(ADC3_value[0],-max_value,max_value,0,63);
	    SSD1306_UpdateScreen();
	    ++idx;
	 }  	 
      }

      unsigned char str[255];

      if(ADC3_ready){
	 sprintf(str,"R%d,%d,%d,%d,%d,",micros,ADC3_value[0],ADC3_value[1],ADC3_value[2],ADC3_value[3]);  
      }

      if(USB_VCP_GetStatus() == USB_VCP_CONNECTED) {
	 if(sn == 1){
	    uint8_t c;
	    if (USB_VCP_Getc(&c) == USB_VCP_DATA_OK) {
	       /* Return data back */
	       //TM_USB_VCP_Putc(c);
	       if(c == 'A' && id_state == 0){
		  ++id_state;
	       }
	       if(c == 'C' && id_state == 1){
		  ++id_state;
	       }
	       if(c == 'K' && id_state == 2){
		  ++id_state;
		  USB_VCP_Puts("ACK");
		  sn = 0;

	       }
	    }
	 }
	 else{
	    uint8_t c;
	    if (USB_VCP_Getc(&c) == USB_VCP_DATA_OK) {
	       /* Return data back */
	       //TM_USB_VCP_Putc(c);
	       if(c == 'G' && recheck_state == 0){
		  ++recheck_state;
	       }
	       if(c == 'O' && recheck_state == 1){
		  ++recheck_state;
	       }
	       if(c == 'T' && recheck_state == 2){
		  ++recheck_state;
		  USB_VCP_Puts("GOT");
	       }
	    }
	 }

	 if(recheck_state == 3){
	    USB_VCP_Puts(str);
	 }
	 blue_flag ^= true;
      }
      Delay(1000);
   }

   return(0); // System will implode
}    

void EXTI0_IRQHandler(void)
{
   if(EXTI_GetITStatus(EXTI_Line0) != RESET){
      ++mode;
      if(mode > ECG_SIGNAL_MODE){
	 mode = SPO2_SIGNAL_MODE;
      }
   }

   clear_flag = true;

   EXTI_ClearITPendingBit(EXTI_Line0);
   EXTI_ClearFlag(EXTI_Line0);
}
