TARGET=main.hex
EXECUTABLE=main.elf

CC=arm-none-eabi-gcc
LD=arm-none-eabi-gcc
AR=arm-none-eabi-ar
AS=arm-none-eabi-as
CP=arm-none-eabi-objcopy
OD=arm-none-eabi-objdump

BIN=$(CP) -O ihex 

DEFS = -DUSE_STDPERIPH_DRIVER -DSTM32F4XX -DHSE_VALUE=8000000
STARTUP = ../../Libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f40xx.s

MCU = cortex-m4
MCFLAGS = -mcpu=$(MCU) -mthumb -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb-interwork -std=c99
STM32_INCLUDES = -I../../Libraries/CMSIS/Device/ST/STM32F4xx/Include/ \
	-I../../Libraries/CMSIS/Include/ \
	-I../../Libraries/STM32F4xx_StdPeriph_Driver/inc/

OPTIMIZE       = -Os

CFLAGS	= $(MCFLAGS)  $(OPTIMIZE)  $(DEFS) -I. -I./ $(STM32_INCLUDES)  -Wl,-T,STM32F405.ld
AFLAGS	= $(MCFLAGS) 
#-mapcs-float use float regs. small increase in code size

SRC = main.c \
	stm32f4xx_it.c \
	system_stm32f4xx.c \
	./usb_cdc_device/*.c \
	stm32f4_fonts.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/misc.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_can.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_crc.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_aes.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_des.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_tdes.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dac.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dbgmcu.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dcmi.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fsmc.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_md5.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_sha1.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_iwdg.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_pwr.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rng.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rtc.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c \
	../../Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_wwdg.c 

OBJDIR = .
OBJ = $(SRC:%.c=$(OBJDIR)/%.o) 
OBJ += Startup.o

all: $(TARGET)

$(TARGET): $(EXECUTABLE)
	$(CP) -O ihex $^ $@

$(EXECUTABLE): $(SRC) $(STARTUP)
	$(CC) $(CFLAGS) $^ -lm -lc -lnosys -o $@

clean:
	rm -f Startup.lst  $(TARGET)  $(TARGET).lst $(OBJ) $(AUTOGEN)  $(TARGET).out  $(TARGET).hex  $(TARGET).map \
	 $(TARGET).dmp  $(TARGET).elf
