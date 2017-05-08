#ifndef _debug_usart_
#define _debug_usart_

#include <stm32f7xx.h>

#define DBG_USART_GPIO_RCC()	__HAL_RCC_GPIOC_CLK_ENABLE()
#define DBG_USART_USART_RCC()	__HAL_RCC_USART6_CLK_ENABLE()
#define DMAx_CLK_ENABLE()		__HAL_RCC_DMA2_CLK_ENABLE()

#define DBG_USART_baud			115200
#define DBG_USARTx 				USART6
#define DBG_USART_AF			GPIO_AF8_USART6
#define DBG_USART_GPIO			GPIOC
#define DBG_USART_TXPIN			GPIO_PIN_6
#define DBG_USART_RXPIN			GPIO_PIN_7

// Definition for USARTx's DMA
#define USARTx_TX_DMA_STREAM	DMA2_Stream7
#define USARTx_RX_DMA_STREAM	DMA2_Stream1
#define USARTx_TX_DMA_CHANNEL	DMA_CHANNEL_5
#define USARTx_RX_DMA_CHANNEL	DMA_CHANNEL_5

#define USARTx_DMA_TX_IRQn		DMA2_Stream7_IRQn
#define USARTx_DMA_RX_IRQn		DMA2_Stream1_IRQn
#define USARTx_DMA_TX_IRQHandler	DMA2_Stream7_IRQHandler
#define USARTx_DMA_RX_IRQHandler	DMA2_Stream1_IRQHandler

#define USARTx_IRQn				USART6_IRQn
#define USARTx_IRQHandler		USART6_IRQHandler

// Size of DMA buffers
#define TXBUFFERSIZE			(COUNTOF(aTxBuffer) - 1)
#define RXBUFFERSIZE			TXBUFFERSIZE

#define DBG_USART_TIMEOUT	0x40000 /* a long time */
#define USART_FIFO_COUNT	0x8
#define USART_MAX_LENGTH	0x20 // max print string is 32 chars

// Setup functions & DMA/IT Handlers
void Debug_USART_Init(void);
void USARTx_DMA_RX_IRQHandler(void);
void USARTx_DMA_TX_IRQHandler(void);
void HAL_USART_TxCpltCallback(USART_HandleTypeDef *husart);
void HAL_USART_RxCpltCallback(USART_HandleTypeDef *husart);
void USARTx_IRQHandler(void);

// Communication Fns
void Debug_USART_putc(unsigned char c);
void Debug_USART_printf(char *s);
void Debug_USART_putn(uint32_t n);
void Debug_USART_putn8(uint8_t n);

// Next Gen Fn Calls
void DB_print_var(char* name, uint32_t n, uint8_t ret_flag);

#endif