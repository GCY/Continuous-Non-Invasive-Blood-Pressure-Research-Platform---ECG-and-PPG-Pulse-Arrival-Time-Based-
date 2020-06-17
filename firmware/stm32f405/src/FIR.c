#include "FIR.h"

float FIR_filter(float input,FIRInfo *info)
{
   float *coeff     = info->coeff;
   float *coeff_end = info->coeff + info->taps;

   float *buf_val = info->buffer + info->offset;

   *buf_val = input;
   float output_ = 0;

   while(buf_val >= info->buffer){
      output_ += *buf_val-- * *coeff++;
   }

   buf_val = info->buffer + info->taps-1;

   while(coeff < coeff_end){
      output_ += *buf_val-- * *coeff++;
   }

   if(++(info->offset) >= info->taps){
      info->offset = 0;
   }

   return output_;
}

void FIR_reset_buffer(FIRInfo *info)
{
   memset(info->buffer, 0, sizeof(float) * info->taps);
   info->offset = 0;
}
