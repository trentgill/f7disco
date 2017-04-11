#include "disco_hw.h"

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

}

void Disco_HW_Demo(void)
{
	if(BSP_PB_GetState(0)){
		BSP_LED_Toggle(LED1);
	} else {
		BSP_LED_Off(LED1);
	}
	
	BSP_LED_Toggle(LED2);
}