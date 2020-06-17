#ifndef __DEFINE__
#define __DEFINE__

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
   float value;
   int32_t index;
}SignalPoint;

extern const uint32_t SAMPLING_RATE;
extern uint32_t ADC3_value[4];
extern bool  ADC3_ready;
extern uint32_t adc_value[4];

#endif
