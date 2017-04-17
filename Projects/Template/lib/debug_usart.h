#ifndef _debug_usart_
#define _debug_usart_

#include <stm32f7xx.h>

#define DBG_USART_GPIO_RCC()	__HAL_RCC_GPIOC_CLK_ENABLE()
#define DBG_USART_USART_RCC()	__HAL_RCC_USART6_CLK_ENABLE()

#define DBG_USART_baud		115200
#define DBG_USARTx 			USART6
#define DBG_USART_AF		GPIO_AF8_USART6
#define DBG_USART_GPIO		GPIOC
#define DBG_USART_TXPIN		GPIO_PIN_6
#define DBG_USART_RXPIN		GPIO_PIN_7
#define DBG_USART_PINMASK	(DBG_USART_TXPIN | DBG_USART_RXPIN)

#define DBG_USART_TIMEOUT	0x40000 /* a long time */

void Debug_USART_Init(void);
void Debug_USART_putc(unsigned char c);
void Debug_USART_printf(char *s);
void Debug_USART_putn(uint32_t n);
void Debug_USART_putn8(uint8_t n);

#endif