TARGET=main
EXECUTABLE=main.elf

CUBE=../STM32Cube_FW_F7_V1.6.0/Drivers
HALS=STM32F7xx_HAL_Driver/Src
WRLIB=../../wrLib
LUAS=../../lua-5.3.4/src
# $(PRJ_DIR) = /f7disco

CC=arm-none-eabi-gcc
LD=arm-none-eabi-gcc
AR=arm-none-eabi-ar
AS=arm-none-eabi-as
CP=arm-none-eabi-objcopy
OD=arm-none-eabi-objdump

# BIN=$(CP) -O ihex 
BIN = $(TARGET).bin

DEFS = -DUSE_STDPERIPH_DRIVER -DSTM32F7XX -DARM_MATH_CM7 -DHSE_VALUE=25000000
STARTUP = $(CUBE)/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f769xx.s

MCU = cortex-m4
MCFLAGS = -mcpu=$(MCU) -mthumb -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb-interwork -std=c99
STM32_INCLUDES = \
	-I$(CUBE)/CMSIS/Device/ST/STM32F7xx/Include/ \
	-I$(CUBE)/CMSIS/Include/ \
	-I$(CUBE)/BSP/Components/ \
	-I$(CUBE)/BSP/STM32F769I-Discovery/ \
	-I$(CUBE)/STM32F7xx_HAL_Driver/Inc/ \
	-I$(LUALIB)/include/

# note: /usr/include/luaconf.h has the "include lua5.3-deb-multiarch.h" line commented out

OPTIMIZE       = -O3

CFLAGS	= $(MCFLAGS)  $(OPTIMIZE)  $(DEFS) -I. -I./ $(STM32_INCLUDES)  -Wl,-T,stm32_flash.ld

AFLAGS	= $(MCFLAGS) -mapcs-float
#-mapcs-float use float regs. small increase in code size

SRC = main.c \
	stm32f7xx_it.c \
	system_stm32f7xx.c \
	stm32f7xx_hal_msp.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal_dfsdm.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal_cortex.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal_rcc.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal_rcc_ex.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal_gpio.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal_pwr_ex.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal_i2c.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal_ltdc.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal_ltdc_ex.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal_dma.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal_dma2d.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal_dsi.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal_sdram.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal_sai.c \
	$(CUBE)/$(HALS)/stm32f7xx_hal_usart.c \
	$(CUBE)/$(HALS)/stm32f7xx_ll_fmc.c \
	$(CUBE)/BSP/STM32F769I-Discovery/stm32f769i_discovery.c \
	$(CUBE)/BSP/STM32F769I-Discovery/stm32f769i_discovery_audio.c \
	$(CUBE)/BSP/STM32F769I-Discovery/stm32f769i_discovery_lcd.c \
	$(CUBE)/BSP/STM32F769I-Discovery/stm32f769i_discovery_sdram.c \
	$(CUBE)/BSP/Components/otm8009a/otm8009a.c \
	$(CUBE)/BSP/Components/wm8994/wm8994.c \
	$(WRLIB)/wrLpGate.c \
	$(WRLIB)/wrFuncGen.c \
	$(WRLIB)/wrMath.c \
	lib/*.c \



OBJDIR = .
OBJ = $(SRC:%.c=$(OBJDIR)/%.o) 
OBJ += Startup.o

all: $(TARGET).hex

$(TARGET).hex: $(EXECUTABLE)
	$(CP) -O ihex $^ $@

$(EXECUTABLE): $(SRC) $(STARTUP)
	$(CC) -g $(CFLAGS) $^ -lm -lc -lnosys -o $@

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
		$(PRJ_DIR)/$(BIN)

fsk-wav: $(BIN)
	cd .. && python stm-audio-bootloader/fsk/encoder.py \
		-s 48000 -b 16 -n 8 -z 4 -p 256 -g 16384 -k 1800 \
		$(PRJ_DIR)/$(BIN)