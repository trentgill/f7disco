#include <string.h>
#include "debug_usart.h"
#include "disco_hw.h"
#include "disco_term.h"
#include "str_buffer.h"

// public declaration for use in transmit function
USART_HandleTypeDef handusart;
// __IO ITStatus uReady = RESET;
unsigned char USART_Rx_Buf[51];

str_buffer_t str_buf; // circular string buffer

void Debug_USART_Init(void)
{
	handusart.Instance = DBG_USARTx;

	handusart.Init.BaudRate 		= DBG_USART_baud;
	handusart.Init.WordLength 		= USART_WORDLENGTH_8B;
	handusart.Init.StopBits 		= USART_STOPBITS_1;
	handusart.Init.Parity 			= USART_PARITY_NONE;
	handusart.Init.Mode 			= USART_MODE_TX_RX;
	HAL_USART_Init( &handusart );

	str_buffer_init(&str_buf, 256); // fifo for DMA buffer
}

// LOW LEVEL USART HARDWARE CONFIGURATION FUNCTION
void HAL_USART_MspInit(USART_HandleTypeDef *hu )
{
	static DMA_HandleTypeDef hdma_tx;
	static DMA_HandleTypeDef hdma_rx;

	DBG_USART_USART_RCC();
	DBG_USART_GPIO_RCC();
	DMAx_CLK_ENABLE();

	GPIO_InitTypeDef gpio;
	gpio.Pin = DBG_USART_TXPIN;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH; // _SPEED_HIGH ?
	gpio.Alternate = DBG_USART_AF;
	HAL_GPIO_Init(DBG_USART_GPIO, &gpio);

	gpio.Pin = DBG_USART_RXPIN;
	HAL_GPIO_Init(DBG_USART_GPIO, &gpio);

	// Configure DMA
	hdma_tx.Instance				= USARTx_TX_DMA_STREAM;
	hdma_tx.Init.Channel			= USARTx_TX_DMA_CHANNEL;
	hdma_tx.Init.Direction			= DMA_MEMORY_TO_PERIPH;
	hdma_tx.Init.PeriphInc			= DMA_PINC_DISABLE;
	hdma_tx.Init.MemInc				= DMA_MINC_ENABLE;
	hdma_tx.Init.PeriphDataAlignment= DMA_PDATAALIGN_BYTE;
	hdma_tx.Init.MemDataAlignment	= DMA_MDATAALIGN_BYTE;
	hdma_tx.Init.Mode 				= DMA_NORMAL;
	hdma_tx.Init.Priority 			= DMA_PRIORITY_LOW;
	HAL_DMA_Init( &hdma_tx );
	__HAL_LINKDMA( hu, hdmatx, hdma_tx ); // Associate DMA to USART handle

	hdma_rx.Instance				= USARTx_RX_DMA_STREAM;
	hdma_rx.Init.Channel			= USARTx_RX_DMA_CHANNEL;
	hdma_rx.Init.Direction			= DMA_PERIPH_TO_MEMORY;
	hdma_rx.Init.PeriphInc			= DMA_PINC_DISABLE;
	hdma_rx.Init.MemInc				= DMA_MINC_ENABLE;
	hdma_rx.Init.PeriphDataAlignment= DMA_PDATAALIGN_BYTE;
	hdma_rx.Init.MemDataAlignment	= DMA_MDATAALIGN_BYTE;
	hdma_rx.Init.Mode 				= DMA_NORMAL;
	hdma_rx.Init.Priority 			= DMA_PRIORITY_HIGH;
	HAL_DMA_Init( &hdma_rx );
	__HAL_LINKDMA( hu, hdmarx, hdma_rx ); // Associate DMA to USART handle

	// Configure NVIC for DMA
	HAL_NVIC_SetPriority(USARTx_DMA_TX_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(USARTx_DMA_TX_IRQn);

	HAL_NVIC_SetPriority(USARTx_DMA_RX_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USARTx_DMA_RX_IRQn);

	HAL_NVIC_SetPriority(USARTx_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(USARTx_IRQn);

	HAL_USART_Receive_DMA( &handusart, USART_Rx_Buf, 5);
}

void USARTx_DMA_RX_IRQHandler(void)
{
	// BSP_LED_On(LED1);
	HAL_DMA_IRQHandler(handusart.hdmarx);
}
void USARTx_DMA_TX_IRQHandler(void)
{
	HAL_DMA_IRQHandler(handusart.hdmatx);
}
void HAL_USART_TxCpltCallback(USART_HandleTypeDef *husart)
{
	// NOT FIRING?!
}

void HAL_USART_TxHalfCpltCallback(USART_HandleTypeDef *husart)
{
	// NEEDS TO GO IN COMPLETE CALLBACK
	if(str_buffer_notempty( &str_buf )){
		char* so = str_buffer_dequeue( &str_buf );
		HAL_USART_Transmit_DMA( &handusart, so, strlen(so) );
	}
}
void HAL_USART_RxCpltCallback(USART_HandleTypeDef *husart)
{
	//
}
void USARTx_IRQHandler(void)
{
	HAL_USART_IRQHandler( &handusart );
}




// Communication Functions
void Debug_USART_printf(char *s)
{
	str_buffer_enqueue( &str_buf, s );
	char* so = str_buffer_dequeue( &str_buf );
	HAL_USART_Transmit_DMA( &handusart, so, strlen(so) );
}
void Debug_USART_putc(unsigned char c)
{
	static char str[4] = "0\n\r\0";
	str[0] = c;
	HAL_USART_Transmit_DMA( &handusart, str, 1 );
}
void Debug_USART_putn(uint32_t n)
{
	// declared static as DMA just points directly to it
	static char str[13] = "0xFFFFFFFF\n\r\0";
	uint32_t temp;
	for(int8_t i=7; i>=0; i--){
		temp = n >> (i<<2);
		temp &= 0x0000000F; // mask lowest nibble
		if(temp<10) { // numeric
			str[9-i] = 48+(uint8_t)temp;
		} else { // alpha
			str[9-i] = 55+(uint8_t)temp;
		}
	}
	Debug_USART_printf(str);
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

// New school func calls
void DB_print_var(char* name, uint32_t n, uint8_t ret_flag)
{
	static char str[24];
	uint8_t len = strlen(name);
	if(len > 10) { len = 10; }
	uint8_t i;
	for(i=0; i<len; i++){
		str[i] = name[i];
	}
	// add separator
	strcpy(&str[i], ": 0x");
	i += 4;
	// print hex readout
	uint32_t temp;
	for(int8_t j=7; j>=0; j--){
		temp = n >> (j<<2);
		temp &= 0x0000000F; // mask lowest nibble
		if(temp<10) { // numeric
			str[(i+7)-j] = 48+(uint8_t)temp;
		} else { // alpha
			str[(i+7)-j] = 55+(uint8_t)temp;
		}
	}
	if(ret_flag){
		strcpy(&str[i+8], "\n\r\0"); // carriage return
	} else {
		strcpy(&str[i+8], ", \0"); // add space
	}
	Debug_USART_printf(str); // call basic print function
}









