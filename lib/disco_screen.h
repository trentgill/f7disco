#ifndef _disco_screen_
#define _disco_screen_

// do these need absolute references?
#include "../../../../STM32Cube_FW_F7_V1.6.0/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_lcd.h"
#include "../../../../STM32Cube_FW_F7_V1.6.0/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_sdram.h"

void Disco_Screen_Init(void);
void Disco_Screen_Loop(void);

// IRQ 'STRONG' definition
void DSI_IRQHandler(void);


#endif