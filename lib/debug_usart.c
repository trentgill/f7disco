#include <string.h>
#include "debug_usart.h"
#include "disco_hw.h"

// public declaration for use in transmit function
USART_HandleTypeDef handusart;

void Debug_USART_Init(void)
{
	handusart.Instance = DBG_USARTx;

	handusart.Init.BaudRate 		= DBG_USART_baud;
	handusart.Init.WordLength 		= USART_WORDLENGTH_8B;
	handusart.Init.StopBits 		= USART_STOPBITS_1;
	handusart.Init.Parity 			= USART_PARITY_NONE;
	handusart.Init.Mode 			= USART_MODE_TX;
	HAL_USART_Init( &handusart );
}

void HAL_USART_MspInit(USART_HandleTypeDef *hu )
{
	// LOW LEVEL USART HARDWARE CONFIGURATION FUNCTION
	
	DBG_USART_USART_RCC();
	DBG_USART_GPIO_RCC();

	GPIO_InitTypeDef gpio;
	// gpio.Pin = DBG_USART_PINMASK;
	gpio.Pin = DBG_USART_TXPIN;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;
	gpio.Alternate = DBG_USART_AF;
	HAL_GPIO_Init(DBG_USART_GPIO, &gpio);

	gpio.Pin = DBG_USART_RXPIN;
	HAL_GPIO_Init(DBG_USART_GPIO, &gpio);

}

void Debug_USART_putc(unsigned char c)
{
	while( HAL_USART_GetState( &handusart ) != HAL_USART_STATE_READY );
	HAL_USART_Transmit( &handusart, &c, 1, DBG_USART_TIMEOUT );
}

void Debug_USART_printf(char *s)
{
	while( HAL_USART_GetState( &handusart ) != HAL_USART_STATE_READY );
	HAL_USART_Transmit( &handusart, s, strlen(s), DBG_USART_TIMEOUT );
}

void Debug_USART_putn(uint32_t n)
{
	uint8_t temp;
	Debug_USART_printf("\n\r0x");
	for(int8_t i=7; i>=0; i--){
		temp = n >> (i<<2);
		temp &= 0x0000000F; // mask lowest nibble
		if(temp<10) { // numeric
			Debug_USART_putc((char)48+temp);
		} else { // alpha
			Debug_USART_putc((char)55+temp);
		}
	}
}

void Debug_USART_putn8(uint8_t n)
{
	uint8_t temp;
	Debug_USART_printf("\n\r");
	for(int8_t i=1; i>=0; i--){
		temp = n >> (i<<2);
		temp &= 0x0F; // mask lowest nibble
		if(temp<10){ // numeric
			Debug_USART_putc((char)48+temp);
		} else { // alpha
			Debug_USART_putc((char)55+temp);
		}
	}
}
