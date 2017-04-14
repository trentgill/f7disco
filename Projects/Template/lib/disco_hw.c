#include "disco_hw.h"
#include "debug_usart.h"
// #include "disco_screen.h"

//
void Disco_HW_Init(void)
{
	// LEDs
	BSP_LED_Init(LED1);
	BSP_LED_Init(LED2);
	// BSP_LED_On(LED1); // turn it on!
	// BSP_LED_On(LED2); // turn it on!

	// Buttons
	BSP_PB_Init(0, BUTTON_MODE_GPIO);

	// Screen
	// Disco_Screen_Init();

}

void Disco_HW_Loop(void)
{
	static uint8_t lastButton;

	uint8_t press = BSP_PB_GetState(0);

	if(lastButton != press){
		Debug_USART_printf("push");
		// BSP_LED_Toggle(LED1);

	} else {
		BSP_LED_Off(LED1);
	}
	
	BSP_LED_Toggle(LED2);

	lastButton = press;
	// Disco_Screen_Loop();
}	