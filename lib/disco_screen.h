#pragma once

#include <stm32f769i_discovery_lcd.h>
#include <stm32f769i_discovery_sdram.h>

void Disco_Screen_Init(void);

// IRQ 'STRONG' definition
void DSI_IRQHandler(void);
