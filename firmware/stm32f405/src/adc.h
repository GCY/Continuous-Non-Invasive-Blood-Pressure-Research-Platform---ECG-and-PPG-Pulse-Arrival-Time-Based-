#ifndef __ADC__
#define __ADC__

#include <stdbool.h>

#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_dma.h>
#include <stm32f4xx_adc.h>
#include <stm32f4xx_tim.h>

#include "define.h"


#define ADC3_DR_ADDRESS (uint32_t)ADC3_BASE+0x4C

extern void ADC_4_Channel_IO_Init();
extern void ADC_4_Channel_DMA_Init();
extern void ADC_4_Channel_AD_Init();
extern void TIM4_init();
extern void ADC_4_Channel_ReInit();
extern void ADC_4_Channel_Init();
extern void DMA2_Stream0_IRQHandler();

extern void Init_ADC();

#endif
