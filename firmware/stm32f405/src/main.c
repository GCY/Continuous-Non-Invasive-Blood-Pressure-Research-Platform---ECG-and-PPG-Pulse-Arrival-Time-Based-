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

//#include "tiny_printf.h"
#include "stm32f4_fonts.h"

#define __FPU_PRESENT
#define __FPU_USED

volatile const uint32_t SECOND = 1000000;

volatile uint32_t TimingDelay;

volatile uint32_t micros = 0;

volatile uint32_t last_screen_update = 0;

enum{
   SPO2_SIGNAL_MODE = 1,
   SPO2_VALUE_MODE,
   ECG_SIGNAL_MODE
};
volatile bool clear_flag = false;
volatile uint32_t mode = SPO2_SIGNAL_MODE;

uint32_t ORG_IR = 0;
uint32_t ORG_RED = 0;
int32_t IRac = 0;
int32_t REDac = 0;
Filter_Data irf = {0};
Filter_Data redf = {0};   
uint32_t irACValueSqSum = 0;
uint32_t redACValueSqSum = 0;
uint16_t samplesRecorded = 0;
uint16_t pulsesDetected = 0;
uint32_t currentSaO2Value = 0;  

volatile uint32_t max_set_time = 0;
volatile int32_t max_value = 100;
volatile int32_t current_max = 100;

volatile uint32_t optical_read_time = 0;
volatile bool optical_ready = false;
volatile bool optical_process_flag = false;

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

void EXTILine15_10_Config(void)
{
   EXTI_InitTypeDef   EXTI_InitStructure;
   GPIO_InitTypeDef   GPIO_InitStructure;
   NVIC_InitTypeDef   NVIC_InitStructure;

   /* Enable GPIOB clock */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
   /* Enable SYSCFG clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

   /* Configure PB0 pin as input floating */
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
   GPIO_Init(GPIOB, &GPIO_InitStructure);

   /* Connect EXTI Line0 to PB0 pin */
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource12);

   /* Configure EXTI Line0 */
   EXTI_InitStructure.EXTI_Line = EXTI_Line12;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);

   /* Enable and set EXTI Line0 Interrupt to the lowest priority */
   NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
}

void MAX3010x_SignalProcess()
{
   Read_MAX3010x();

   ORG_IR = IR;
   ORG_RED = RED;

   IRac = DCRemove(IR,&IRcw);
   REDac = DCRemove(RED,&REDcw);

   IRac = MeanDiff(IRac);
   REDac = MeanDiff(REDac);

   IRac = LowPassButterworthFilter(IRac,&irf);
   REDac = LowPassButterworthFilter(REDac,&redf);


   irACValueSqSum += IRac * IRac;
   redACValueSqSum += REDac * REDac;
   samplesRecorded++;  
   if(/*detectPulse(IRac)*/true){
      pulsesDetected++;


      float red_log_rms = log( sqrt(redACValueSqSum/samplesRecorded) );
      float ir_log_rms = log( sqrt(irACValueSqSum/samplesRecorded) );
      float ratioRMS = 0.0f;
      if(red_log_rms != 0.0f && ir_log_rms != 0.0f){
	 ratioRMS = red_log_rms / ir_log_rms;
      }
      currentSaO2Value = 110.0f - 14.0f * ratioRMS;	// SPo2 value by pulse-rate
      if(currentSaO2Value > 99){currentSaO2Value = 99;}
      else if(currentSaO2Value < 70){currentSaO2Value = 70;}


      if(pulsesDetected % RESET_SPO2_EVERY_N_PULSES == 0){
	 irACValueSqSum = 0;
	 redACValueSqSum = 0;
	 samplesRecorded = 0;
      }     
   }


   BalanceIntesities();   
}

int main(void)
{
   if(SysTick_Config(SystemCoreClock / 1000 / 1000)){
      while(1){}
   }

   EXTILine15_10_Config();
   Delay(5000);
   EXTILine0_Config();
   Delay(5000);

   USBVCPInit();
   Delay(5000);

   Init_MAX3010x();
   Delay(5000);
   MAX3010x_SignalProcess();
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
   Delay(SECOND);
   SSD1306_Fill(0xFF);
   SSD1306_UpdateScreen();
   Delay(SECOND);
   SSD1306_Fill(0x00);
   SSD1306_UpdateScreen();  
   Delay(SECOND);   

   SSD1306_GotoXY(3, 4);
   SSD1306_Puts("NICBP v2", &Font_11x18, 0xFF);
   SSD1306_GotoXY(3, 25);
   SSD1306_Puts("20190817", &Font_11x18, 0xFF);
   SSD1306_GotoXY(3, 45);
   SSD1306_Puts("TonyGUO", &Font_11x18, 0xFF);
   SSD1306_UpdateScreen();  
   Delay(SECOND);
   SSD1306_Fill(0x00);
   SSD1306_UpdateScreen();

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
/*
      if((micros - optical_read_time) > 20000){
	 optical_read_time = micros;
	 optical_ready = true;
      }
*/
      if(USB_VCP_GetStatus() == USB_VCP_CONNECTED){
	 if(micros - last_screen_update > SECOND){
	    char str[255] = "";
	    SSD1306_GotoXY(20, 25);
	    SSD1306_Puts("USB Mode", &Font_11x18, 0xFF);	    
	    SSD1306_UpdateScreen();
	    last_screen_update = micros;
	 }

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
	    if(ADC3_ready){
	       ADC3_ready = false;
	       unsigned char str[255] = {0};
	       //sprintf(str,"R%4d,%4d,%4d,%4d,%4d,",micros,ADC3_value[0],ADC3_value[1],ADC3_value[2],ADC3_value[3]);
	       sprintf(str,"D%4d,%6d,%6d, \n",ADC3_value[0],ORG_IR,ORG_RED);
	       USB_VCP_Puts(str);
	       if(optical_ready){
		  optical_ready = false;
		  MAX3010x_SignalProcess();
	       }	       
	    } 
	    //else if(optical_ready/*Read_MAX30100_INT()*/){
	    //   optical_ready = false;
	    //   MAX3010x_SignalProcess();
	       /*
		  unsigned char str[255];
		  sprintf(str,"O%6d, \n",RED);
		  USB_VCP_Puts(str);*/
	    //}
	 }
	 blue_flag ^= true;
      }
      else{
	 if(mode == SPO2_SIGNAL_MODE){

	    MAX3010x_SignalProcess();

	    if( (micros - max_set_time) > (SECOND * 1.5f)){
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

	    MAX3010x_SignalProcess();

	    if(clear_flag){
	       SSD1306_Fill(0x00);
	       SSD1306_UpdateScreen();
	       clear_flag = false;
	    }	 

	    unsigned char spo2_str[255];
	    sprintf(spo2_str,"SpO2 : %2d",currentSaO2Value);
	    SSD1306_GotoXY(3, 25);
	    SSD1306_Puts(spo2_str, &Font_11x18, 0xFF);
	    SSD1306_UpdateScreen();

	 }
	 else if(mode == ECG_SIGNAL_MODE && ADC3_ready){
	    ADC3_ready = false;
	    if( (micros - max_set_time) > (SECOND * 1.5f)){
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

      }
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

void EXTI15_10_IRQHandler(void)
{
   if(EXTI_GetITStatus(EXTI_Line12) != RESET){
      optical_ready = true;
   }

   EXTI_ClearITPendingBit(EXTI_Line12);
   EXTI_ClearFlag(EXTI_Line12);   
}
