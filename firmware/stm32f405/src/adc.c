#include "adc.h"

void ADC_4_Channel_IO_Init( void )
{
   GPIO_InitTypeDef GPIO_InitStructure;

   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;;//ADC0~3
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init( GPIOC, &GPIO_InitStructure );
}

void ADC_4_Channel_DMA_Init( void )
{
   DMA_InitTypeDef DMA_InitStructure;
   NVIC_InitTypeDef   NVIC_InitStructure;

   DMA_InitStructure.DMA_Channel = DMA_Channel_2;
   DMA_InitStructure.DMA_PeripheralBaseAddr = ( uint32_t )ADC3_DR_ADDRESS;
   DMA_InitStructure.DMA_Memory0BaseAddr = ( uint32_t ) &ADC3_value;
   DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
   DMA_InitStructure.DMA_BufferSize = 4; 
   DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
   DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
   DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Word;
   DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
   DMA_InitStructure.DMA_Priority = DMA_Priority_High;
   DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
   DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
   DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
   DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

   DMA_Init( DMA2_Stream0, &DMA_InitStructure );

   NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

   DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);

   DMA_Cmd( DMA2_Stream0, ENABLE );
}

void ADC_4_Channel_AD_Init( void )
{
   ADC_InitTypeDef ADC_InitStructure;
   ADC_CommonInitTypeDef ADC_CommonInitStructure;

   ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
   ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
   ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
   ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_SampleTime_3Cycles;
   ADC_CommonInit( &ADC_CommonInitStructure );

   ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
   ADC_InitStructure.ADC_ScanConvMode = ENABLE;
   ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
   ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
   ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
   ADC_InitStructure.ADC_NbrOfConversion = 4;
   ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T4_CC4;

   ADC_Init( ADC3, &ADC_InitStructure );

   ADC_RegularChannelConfig( ADC3, ADC_Channel_10, 1, ADC_SampleTime_15Cycles );
   ADC_RegularChannelConfig( ADC3, ADC_Channel_11, 2, ADC_SampleTime_15Cycles );
   ADC_RegularChannelConfig( ADC3, ADC_Channel_12, 3, ADC_SampleTime_15Cycles );
   ADC_RegularChannelConfig( ADC3, ADC_Channel_13, 4, ADC_SampleTime_15Cycles );

   ADC_DMARequestAfterLastTransferCmd( ADC3, ENABLE );
   ADC_DMACmd( ADC3, ENABLE );
   ADC_Cmd( ADC3, ENABLE );
}

void TIM4_init( void )
{
   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   TIM_OCInitTypeDef  TIM_OCInitStructure;


   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

   // Timer Freq = 84000000 / (1000000/SAMPLING_RATE) / 84(Prescaler)
   TIM_TimeBaseStructure.TIM_Period = (1000000/SAMPLING_RATE)-1;
   TIM_TimeBaseStructure.TIM_Prescaler = (84-1);
   TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

   TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

   // PWM Dyty Cycle = 50 / (1000000/SAMPLING_RATE)
   TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
   TIM_OCInitStructure.TIM_Pulse = 50;
   TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;

   TIM_OC4Init(TIM4, &TIM_OCInitStructure);

   TIM_Cmd( TIM4, ENABLE );


}

void ADC_4_Channel_ReInit( void )
{
   RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE );
   RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC3, ENABLE );

   ADC_4_Channel_IO_Init();

   DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, DISABLE);
   DMA_Cmd( DMA2_Stream0, DISABLE );	

   ADC_4_Channel_DMA_Init();	
   ADC_4_Channel_AD_Init();

   TIM4_init();

}

void ADC_4_Channel_Init( void )
{
   RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE );
   RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC3, ENABLE );
   ADC_4_Channel_IO_Init();
   ADC_4_Channel_DMA_Init();
   ADC_4_Channel_AD_Init();
   TIM4_init();
}

void DMA2_Stream0_IRQHandler(void)
{

   DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);

   ADC3_ready	= true;	
}

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


}

