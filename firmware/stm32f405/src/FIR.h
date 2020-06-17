#ifndef __FIR_H__
#define __FIR_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

typedef struct{
   unsigned int taps;
   float *coeff;
   float *buffer;
   unsigned offset;
}FIRInfo;

extern float FIR_filter(float,FIRInfo*);
extern void FIR_reset_buffer(FIRInfo*);

#endif
