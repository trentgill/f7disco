#include "disco_hw.h"
#include "debug_usart.h"
#include "disco_screen.h"
#include "disco_term.h"
//
void Disco_HW_Init(void)
{
	// LEDs
	BSP_LED_Init(LED1);
	BSP_LED_Init(LED2);

	// Buttons
	BSP_PB_Init(0, BUTTON_MODE_GPIO);

	// Screen
	Disco_Screen_Init();
}