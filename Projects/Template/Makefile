TARGET=main
EXECUTABLE=main.elf

# $(HERE) = ./Src

CC=arm-none-eabi-gcc
LD=arm-none-eabi-gcc
AR=arm-none-eabi-ar
AS=arm-none-eabi-as
CP=arm-none-eabi-objcopy
OD=arm-none-eabi-objdump

# BIN=$(CP) -O ihex 
BIN = $(TARGET).bin

DEFS = -DUSE_STDPERIPH_DRIVER -DSTM32F7XX -DARM_MATH_CM7 -DHSE_VALUE=8000000
STARTUP = ../../../STM32Cube_FW_F7_V1.6.0/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f769xx.s

MCU = cortex-m4
MCFLAGS = -mcpu=$(MCU) -mthumb -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb-interwork -std=c99
STM32_INCLUDES = -I../../../STM32Cube_FW_F7_V1.6.0/Drivers/CMSIS/Device/ST/STM32F7xx/Include/ \
	-I../../../STM32Cube_FW_F7_V1.6.0/Drivers/CMSIS/Include/ \
	-I../../../STM32Cube_FW_F7_V1.6.0/Drivers/STM32F7xx_HAL_Driver/Inc/

OPTIMIZE       = -O3

CFLAGS	= $(MCFLAGS)  $(OPTIMIZE)  $(DEFS) -I. -I./ $(STM32_INCLUDES)  -Wl,-T,stm32_flash.ld

AFLAGS	= $(MCFLAGS) -mapcs-float
#-mapcs-float use float regs. small increase in code size

SRC = main.c \
	stm32f7xx_it.c \
	system_stm32f7xx.c \
	stm32f7xx_hal_msp.c \
	../../../STM32Cube_FW_F7_V1.6.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c \
	../../../STM32Cube_FW_F7_V1.6.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c \
	../../../STM32Cube_FW_F7_V1.6.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c \
	../../../STM32Cube_FW_F7_V1.6.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c \
	../../../STM32Cube_FW_F7_V1.6.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c \
	../../../STM32Cube_FW_F7_V1.6.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c.c \
	../../../STM32Cube_FW_F7_V1.6.0/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c \
	../../../STM32Cube_FW_F7_V1.6.0/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery.c \
	lib/*.c

OBJDIR = .
OBJ = $(SRC:%.c=$(OBJDIR)/%.o) 
OBJ += Startup.o

all: $(TARGET).hex

$(TARGET).hex: $(EXECUTABLE)
	$(CP) -O ihex $^ $@

$(EXECUTABLE): $(SRC) $(STARTUP)
	$(CC) $(CFLAGS) $^ -lm -lc -lnosys -o $@

clean:
	rm -f Startup.lst $(TARGET).lst $(OBJ) $(AUTOGEN)  $(TARGET).out  $(TARGET).hex  $(TARGET).map \
	 $(TARGET).dmp  $(EXECUTABLE)

flash: $(BIN)
	st-flash write $(BIN) 0x08000000

$(BIN): $(EXECUTABLE)
	$(CP) -O binary $< $@
	$(OD) -x --syms $< > $(addsuffix .dmp, $(basename $<))
	ls -l $@ $<

wav: fsk-wav

qpsk-wav: $(BIN)
	cd .. && python stm-audio-bootloader/qpsk/encoder.py \
		-t stm32f7 -s 48000 -b 12000 -c 6000 -p 256 \
		$(HERE)/$(BIN)

fsk-wav: $(BIN)
	cd .. && python stm-audio-bootloader/fsk/encoder.py \
		-s 48000 -b 16 -n 8 -z 4 -p 256 -g 16384 -k 1800 \
		$(HERE)/$(BIN)