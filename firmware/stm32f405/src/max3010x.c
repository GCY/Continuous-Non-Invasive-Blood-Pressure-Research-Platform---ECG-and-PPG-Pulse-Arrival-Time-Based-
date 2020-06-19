#include "max3010x.h"

int32_t IRcw = 0;
int32_t REDcw = 0;

uint32_t IR = 0;
uint32_t RED = 0;

uint8_t redLEDCurrent = 5;


int32_t msum = 0;
int32_t mvalues[MEAN_FILTER_SIZE];
int32_t mindex = 0;
int32_t mcount = 0;

volatile uint32_t lastREDLedCurrentCheck = 0;

#if defined(_MAX30102_)
uint32_t decay_counter = 0;
const double decay_coeff = -0.31415;
#endif

uint8_t currentPulseDetectorState = PULSE_IDLE;
uint32_t lastBeatThreshold = 0;
uint32_t currentBPM;
uint32_t valuesBPM[PULSE_BPM_SAMPLE_SIZE] = {0};
uint32_t valuesBPMSum = 0;
uint8_t valuesBPMCount = 0;
uint8_t bpmIndex = 0;

void Init_I2C2()
{
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

    I2C_InitStruct.I2C_ClockSpeed = 100000;
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C2, &I2C_InitStruct);

    I2C_Cmd(I2C2, ENABLE);
}

#if defined(_MAX30100_)
void Init_MAX30100()
{
   I2C_Write(I2C2,MAX30100_ADDRESS,MAX30100_MODE_CONFIG, 0x40);
   Delay(5000);
   I2C_Write(I2C2,MAX30100_ADDRESS,MAX30100_MODE_CONFIG,MAX3010x_MODE_SPO2_HR);
   Delay(5000);
   I2C_Write(I2C2,MAX30100_ADDRESS,MAX30100_INT_ENABLE, 0x10);
   Delay(5000);
   I2C_Write(I2C2,MAX30100_ADDRESS,MAX30100_LED_CONFIG,(i27 << 4) | FIX_IR_CURRENT);
   Delay(5000);
   I2C_Write(I2C2,MAX30100_ADDRESS,MAX30100_SPO2_CONFIG,(sr100<<2) | pw1600);
   Delay(5000);
   I2C_Write(I2C2,MAX30100_ADDRESS,MAX30100_FIFO_WR_PTR,0x00);
   I2C_Write(I2C2,MAX30100_ADDRESS,MAX30100_OVRFLOW_CTR,0x00);
   I2C_Write(I2C2,MAX30100_ADDRESS,MAX30100_FIFO_RD_PTR,0x00);
/*
      I2C_start(I2C2, MAX30100_ADDRESS, I2C_Direction_Transmitter); 
      I2C_write(I2C2, MAX30100_INT_ENABLE); 
      I2C_stop(I2C2); 
      I2C_start(I2C2, MAX30100_ADDRESS, I2C_Direction_Receiver);
      uint8_t int_data;
      int_data = I2C_read_nack(I2C2);
      I2C_stop(I2C2);
      I2C_Write(I2C2,MAX30100_ADDRESS,MAX30100_INT_ENABLE,int_data | 0x10);
      */

}

void Read_MAX30100()
{
   
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
   
   IR = (received_data[0]<<8) | received_data[1];;      // Last IR reflectance datapoint
   RED = (received_data[2]<<8) | received_data[3];;     // Last Red reflectance datapoint   
}

bool Read_MAX30100_INT()
{
   I2C_start(I2C2, MAX30100_ADDRESS, I2C_Direction_Transmitter); 
   I2C_write(I2C2, MAX30100_INT_STATUS); 
   I2C_stop(I2C2); 
   uint8_t received_data[1];
   I2C_start(I2C2, MAX30100_ADDRESS, I2C_Direction_Receiver);
   received_data[0] = I2C_read_nack(I2C2); 
   I2C_stop(I2C2);
   return received_data[0] & 0x10;
}

#elif defined(_MAX30102_)
void Init_MAX30102()
{
   I2C_Write(I2C2,MAX30102_ADDRESS,REG_MODE_CONFIG,0x40);
   Delay(5000);
   I2C_Write(I2C2,MAX30102_ADDRESS,REG_MODE_CONFIG,MAX3010x_MODE_SPO2_HR);
   Delay(5000);
   I2C_Write(I2C2,MAX30102_ADDRESS,REG_LED1_PA, 0x7);
   Delay(5000);
   I2C_Write(I2C2,MAX30102_ADDRESS,REG_LED2_PA, FIX_IR_CURRENT);
   Delay(5000);
   I2C_Write(I2C2,MAX30102_ADDRESS,REG_PILOT_PA, 0x7f);
   Delay(5000);
   I2C_Write(I2C2,MAX30102_ADDRESS,REG_SPO2_CONFIG,(sr100<<2) | pw411);
   Delay(5000);
   /*  I2C_Write(I2C2,MAX30102_ADDRESS,REG_INT_EN_1,0xC0);
       Delay(5000);
       I2C_Write(I2C2,MAX30102_ADDRESS,REG_INT_EN_2,0x00);
       Delay(5000);*/
   I2C_Write(I2C2,MAX30102_ADDRESS,REG_FIFO_WR_PTR,0x00);
   Delay(5000);
   I2C_Write(I2C2,MAX30102_ADDRESS,REG_OVF_COUNTER,0x00);
   Delay(5000);
   I2C_Write(I2C2,MAX30102_ADDRESS,REG_FIFO_RD_PTR,0x00);
   Delay(5000);


   //I2C_Write(I2C2,MAX30102_ADDRESS,REG_INT_ENABLE, ((hr+1) << 4));

   /*
      I2C_start(I2C2, MAX30100_ADDRESS, I2C_Direction_Transmitter); 
      I2C_write(I2C2, MAX30100_INT_ENABLE); 
      I2C_stop(I2C2); 
      I2C_start(I2C2, MAX30100_ADDRESS, I2C_Direction_Receiver);
      uint8_t int_data;
      int_data = I2C_read_nack(I2C2);
      I2C_stop(I2C2);
      I2C_Write(I2C2,MAX30100_ADDRESS,MAX30100_INT_ENABLE,int_data | 0x10);
      */

}

void Read_MAX30102()
{
   I2C_start(I2C2, MAX30102_ADDRESS, I2C_Direction_Transmitter); 
   I2C_write(I2C2, REG_FIFO_DATA); 
   I2C_stop(I2C2); 
   uint8_t received_data[6];
   I2C_start(I2C2, MAX30102_ADDRESS, I2C_Direction_Receiver);
   received_data[0] = I2C_read_ack(I2C2); 
   received_data[1] = I2C_read_ack(I2C2);
   received_data[2] = I2C_read_ack(I2C2);
   received_data[3] = I2C_read_ack(I2C2);
   received_data[4] = I2C_read_ack(I2C2);
   received_data[5] = I2C_read_nack(I2C2); 
   I2C_stop(I2C2);

   RED = (received_data[0]<<16) | (received_data[1]<<8) | received_data[2];      // Last IR reflectance datapoint
   IR = (received_data[3]<<16) | (received_data[4]<<8) | received_data[5];     // Last Red reflectance datapoint  
   RED &= 0x03FFFF;
   IR &= 0x03FFFF;
}
#endif

void Init_MAX3010x()
{
   Init_I2C2();
#if defined(_MAX30100_)
   Init_MAX30100();
#elif defined(_MAX30102_)
   Init_MAX30102();
#endif
}

void Read_MAX3010x()
{
#if defined(_MAX30100_)
   Read_MAX30100();
#elif defined(_MAX30102_)
   Read_MAX30102();
#endif

}

int32_t DCRemove(int32_t value,int32_t *cw)
{
   int32_t oldcw = *cw;
   *cw = value + 0.94 * *cw;
   return *cw - oldcw;
}

int32_t MeanDiff(int32_t M)
{
   int32_t avg = 0;

   msum -= mvalues[mindex];
   mvalues[mindex] = M;
   msum += mvalues[mindex];

   mindex++;
   mindex = mindex % MEAN_FILTER_SIZE;

   if(mcount < MEAN_FILTER_SIZE){
      mcount++;
   }

   avg = msum / mcount;
   return avg - M;
}

int32_t LowPassButterworthFilter(int32_t value,Filter_Data *filter_data)
{  
   filter_data->value[0] = filter_data->value[1];
   //Fs = 100Hz and Fc = 10Hz
   //filter_data->value[1] = (2.452372752527856026e-1 * value) + (0.50952544949442879485 * filter_data->value[0]);

   //Fs = 100Hz and Fc = 4Hz
   filter_data->value[1] = (1.367287359973195227e-1 * value) + (0.72654252800536101020 * filter_data->value[0]); //Very precise butterworth filter 

   return filter_data->value[0] + filter_data->value[1];
}

void BalanceIntesities()
{
#if defined(_MAX30100_)
   if( micros - lastREDLedCurrentCheck >= RED_LED_CURRENT_ADJUSTMENT_NS) {
      //Serial.println( redLedDC - IRLedDC );
      if( IRcw - REDcw > MAGIC_ACCEPTABLE_INTENSITY_DIFF && redLEDCurrent < MAX_CURRENT) {
	 redLEDCurrent++;
	 I2C_Write(I2C2,MAX30100_ADDRESS,MAX30100_LED_CONFIG,(redLEDCurrent << 4) | FIX_IR_CURRENT);
      } 
      else if(REDcw - IRcw > MAGIC_ACCEPTABLE_INTENSITY_DIFF && redLEDCurrent > 0) {
	 redLEDCurrent--;
	 I2C_Write(I2C2,MAX30100_ADDRESS,MAX30100_LED_CONFIG,(redLEDCurrent << 4) | FIX_IR_CURRENT);
      }

      lastREDLedCurrentCheck = micros;
   }
#elif defined(_MAX30102_)
   if( (micros - lastREDLedCurrentCheck) >= RED_LED_CURRENT_ADJUSTMENT_NS){	
      if ( ((IRcw - REDcw) > MAGIC_ACCEPTABLE_INTENSITY_DIFF) && (redLEDCurrent < MAX_CURRENT) ){
	 double decay_factor = exp(decay_coeff*decay_counter);
	 double decay_value = ((IRcw - REDcw)/MAGIC_ACCEPTABLE_INTENSITY_DIFF) * decay_factor;
	 if(decay_value < 1.0f){
	    decay_value = 1.0;
	 }
	 redLEDCurrent+= decay_value;
	 //redLEDCurrent+=((IRcw - REDcw)/MAGIC_ACCEPTABLE_INTENSITY_DIFF); //20190603 TonyGuo
	 if(redLEDCurrent > MAX_CURRENT){redLEDCurrent = MAX_CURRENT;}
	 ////MAX30100_Bus_Write(9, (redLEDCurrent << 4) | MAX30100_LED_CURRENT_50MA);
	 I2C_Write(I2C2,MAX30102_ADDRESS, REG_LED1_PA, redLEDCurrent ); // r_pa=27
	 //I2C_Write(I2C2,MAX30102_ADDRESS, REG_LED2_PA, FIX_IR_CURRENT ); // ir_pa=50
	 //GPIO_ResetBits(GPIOB, GPIO_Pin_12);					
	 ++decay_counter;

      }
      else if ( ((REDcw - IRcw) > MAGIC_ACCEPTABLE_INTENSITY_DIFF) && (redLEDCurrent > 0) ){	
	 double decay_factor = exp(decay_coeff*decay_counter);
	 double decay_value = ((REDcw - IRcw)/MAGIC_ACCEPTABLE_INTENSITY_DIFF) * decay_factor;
	 if(decay_value < 1.0f){
	    decay_value = 1.0;
	 }
	 redLEDCurrent-= decay_value;

	 //redLEDCurrent-=((REDcw - IRcw)/MAGIC_ACCEPTABLE_INTENSITY_DIFF); //20190603 TonyGuo
	 if(redLEDCurrent < 1){redLEDCurrent = 1;}
	 ////MAX30100_Bus_Write(9, (redLEDCurrent << 4) | MAX30100_LED_CURRENT_50MA);
	 I2C_Write(I2C2,MAX30102_ADDRESS, REG_LED1_PA, redLEDCurrent ); // r_pa=27
	 //I2C_Write(I2C2,MAX30102_ADDRESS, REG_LED2_PA, FIX_IR_CURRENT ); // ir_pa=50
	 ++decay_counter;
      }
      else{
	 decay_counter = 0;
      }
      lastREDLedCurrentCheck = micros;
   }
#endif
}

bool detectPulse(uint32_t sensor_value)
{
   static uint32_t prev_sensor_value = 0;
   static uint8_t values_went_down = 0;
   static uint32_t currentBeat = 0;
   static uint32_t lastBeat = 0;

   if(sensor_value > PULSE_MAX_THRESHOLD)
   {
      currentPulseDetectorState = PULSE_IDLE;
      prev_sensor_value = 0;
      lastBeat = 0;
      currentBeat = 0;
      values_went_down = 0;
      lastBeatThreshold = 0;
      return false;
   }

   switch(currentPulseDetectorState)
   {
      case PULSE_IDLE:
	 if(sensor_value >= PULSE_MIN_THRESHOLD) {
	    currentPulseDetectorState = PULSE_TRACE_UP;
	    values_went_down = 0;
	 }
	 break;

      case PULSE_TRACE_UP:
	 if(sensor_value > prev_sensor_value)
	 {
	    currentBeat = micros;
	    lastBeatThreshold = sensor_value;
	 }
	 else
	 {
	    /*
	       if(debug == true) 
	       {
	       Serial.print("Peak reached: ");
	       Serial.print(sensor_value);
	       Serial.print(" ");
	       Serial.println(prev_sensor_value);
	       }*/

	    uint32_t beatDuration = currentBeat - lastBeat;
	    lastBeat = currentBeat;

	    uint32_t rawBPM = 0;
	    if(beatDuration > 0)
	       rawBPM = 60000000 / beatDuration;
	    /*if(debug == true) 
	      Serial.println(rawBPM);*/

	    //This method sometimes glitches, it's better to go through whole moving average everytime
	    //IT's a neat idea to optimize the amount of work for moving avg. but while placing, removing finger it can screw up
	    //valuesBPMSum -= valuesBPM[bpmIndex];
	    //valuesBPM[bpmIndex] = rawBPM;
	    //valuesBPMSum += valuesBPM[bpmIndex];

	    valuesBPM[bpmIndex] = rawBPM;
	    valuesBPMSum = 0;
	    for(int i=0; i<PULSE_BPM_SAMPLE_SIZE; i++)
	    {
	       valuesBPMSum += valuesBPM[i];
	    }
	    /*
	       if(debug == true) 
	       {
	       Serial.print("CurrentMoving Avg: ");
	       for(int i=0; i<PULSE_BPM_SAMPLE_SIZE; i++)
	       {
	       Serial.print(valuesBPM[i]);
	       Serial.print(" ");
	       }

	       Serial.println(" ");
	       }*/

	    bpmIndex++;
	    bpmIndex = bpmIndex % PULSE_BPM_SAMPLE_SIZE;

	    if(valuesBPMCount < PULSE_BPM_SAMPLE_SIZE)
	       valuesBPMCount++;

	    currentBPM = valuesBPMSum / valuesBPMCount;
	    /*if(debug == true) 
	      {
	      Serial.print("AVg. BPM: ");
	      Serial.println(currentBPM);
	      }*/


	    currentPulseDetectorState = PULSE_TRACE_DOWN;

	    return true;
	 }
	 break;

      case PULSE_TRACE_DOWN:
	 if(sensor_value < prev_sensor_value)
	 {
	    values_went_down++;
	 }


	 if(sensor_value < PULSE_MIN_THRESHOLD)
	 {
	    currentPulseDetectorState = PULSE_IDLE;
	 }
	 break;
   }

   prev_sensor_value = sensor_value;
   return false;
}

