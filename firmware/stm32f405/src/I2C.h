#ifndef __I2C__
#define __I2C__

#include <stm32f4xx_i2c.h>

extern void I2C_start(I2C_TypeDef* ,uint8_t ,uint8_t);
extern void I2C_write(I2C_TypeDef* ,uint8_t);
extern uint8_t I2C_read_ack(I2C_TypeDef* );
extern uint8_t I2C_read_nack(I2C_TypeDef* );
extern void I2C_stop(I2C_TypeDef* );
extern void I2C_Write(I2C_TypeDef* ,uint8_t ,uint8_t ,uint8_t);
extern void I2C_WriteMulti(I2C_TypeDef* ,uint8_t ,uint8_t ,uint8_t* ,uint16_t);

#endif
