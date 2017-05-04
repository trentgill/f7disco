#include "disco_codec.h"
#include "disco_hw.h"
#include "debug_usart.h"
#include "dsp_block.h"

// Private defines
#define DSP_BLOCK_SIZE		32
#define PLAY_HALF_BUFF		(DSP_BLOCK_SIZE*2) // stereo
#define PLAY_BUFF_SIZE		(PLAY_HALF_BUFF*2) // double-buff

// Private variables
SAI_HandleTypeDef		SaiHandle;
DMA_HandleTypeDef		hSaiDma;
AUDIO_DrvTypeDef		*audio_drv;

uint16_t				PlayBuff[PLAY_BUFF_SIZE];
__IO int16_t			UpdatePointer = -1;

// Public Functions
void Disco_Codec_Init(void)
{
	RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct;
	// Configure PLLSAI prescalers
		// PLLSAI_VCO: VCO_429M
		// SAI_CLK(first level) = PLLSAI_VCO/PLLSAIQ = 429/2 = 214.5 Mhz
		// SAI_CLK_x = SAI_CLK(first level)/PLLSAIDIVQ = 214.5/19 = 11.289 Mhz
	RCC_PeriphCLKInitStruct.PeriphClockSelection 	= RCC_PERIPHCLK_SAI2;
	RCC_PeriphCLKInitStruct.Sai2ClockSelection 		= RCC_SAI2CLKSOURCE_PLLSAI;
	RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIN 			= 429;
	RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIQ 			= 2;
	RCC_PeriphCLKInitStruct.PLLSAIDivQ 				= 19;
	if( HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) != HAL_OK ){
    	Debug_USART_printf("couldn't init RCC");
	}

	// Initialize SAI
	__HAL_SAI_RESET_HANDLE_STATE(&SaiHandle);

	SaiHandle.Instance = AUDIO_SAIx;

	__HAL_SAI_DISABLE(&SaiHandle);

	SaiHandle.Init.AudioMode		= SAI_MODEMASTER_TX;
	SaiHandle.Init.Synchro			= SAI_ASYNCHRONOUS;
	SaiHandle.Init.OutputDrive		= SAI_OUTPUTDRIVE_ENABLE;
	SaiHandle.Init.NoDivider		= SAI_MASTERDIVIDER_ENABLE;
	SaiHandle.Init.FIFOThreshold	= SAI_FIFOTHRESHOLD_1QF;
	SaiHandle.Init.AudioFrequency	= SAI_AUDIO_FREQUENCY_22K;
	SaiHandle.Init.Protocol			= SAI_FREE_PROTOCOL;
	SaiHandle.Init.DataSize			= SAI_DATASIZE_16;
	SaiHandle.Init.FirstBit			= SAI_FIRSTBIT_MSB;
	SaiHandle.Init.ClockStrobing	= SAI_CLOCKSTROBING_FALLINGEDGE;

	SaiHandle.FrameInit.FrameLength			= 32;
	SaiHandle.FrameInit.ActiveFrameLength	= 16;
	SaiHandle.FrameInit.FSDefinition		= SAI_FS_CHANNEL_IDENTIFICATION;
	SaiHandle.FrameInit.FSPolarity			= SAI_FS_ACTIVE_LOW;
	SaiHandle.FrameInit.FSOffset			= SAI_FS_BEFOREFIRSTBIT;

	SaiHandle.SlotInit.FirstBitOffset	= 0;
	SaiHandle.SlotInit.SlotSize			= SAI_SLOTSIZE_DATASIZE;
	SaiHandle.SlotInit.SlotNumber		= 2; 
	SaiHandle.SlotInit.SlotActive		= (SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1);

	if( HAL_OK != HAL_SAI_Init(&SaiHandle) ){
		Debug_USART_printf("SAI failed init");
	}

	// Enable SAI to generate clock used by audio driver
	__HAL_SAI_ENABLE(&SaiHandle);

	// Initialize audio driver
	if( WM8994_ID != wm8994_drv.ReadID(AUDIO_I2C_ADDRESS) ){
  		Debug_USART_printf("couldn't init audio driver");
	}
	audio_drv = &wm8994_drv;
	audio_drv->Reset(AUDIO_I2C_ADDRESS);
	if( 0 != audio_drv->Init(AUDIO_I2C_ADDRESS, OUTPUT_DEVICE_HEADPHONE, 20, AUDIO_FREQUENCY_22K) ){
		Debug_USART_printf("couldn't reset i2c device");
	}

	// Set DSP_Block Sample Rate
	DSP_Block_Init( SaiHandle.Init.AudioFrequency, DSP_BLOCK_SIZE );

	// Zero the output buffer
	for( uint16_t i=0; i<PLAY_BUFF_SIZE; i++ ){
		PlayBuff[i] = 0;
	}

	// Start playback
	if( 0 != audio_drv->Play(AUDIO_I2C_ADDRESS, NULL, 0) ){
		Debug_USART_printf("coldn't start play function");
	}

	if( HAL_OK != HAL_SAI_Transmit_DMA(&SaiHandle, (uint8_t *)PlayBuff, PLAY_BUFF_SIZE) ){
		Debug_USART_printf("failed to transmit sai dma");
	}
}

// Call DSP Block from main loop
// Could move below into an interrupt to max-prioritize audio
void Disco_Codec_Loop(void)
{
	if( UpdatePointer != -1 ){ // dma waiting flag set
		int position = UpdatePointer; // 0 or PLAY_HALF_BUFF
		UpdatePointer = -1; // reset flag

		// BSP_LED_On(LED1);
		DSP_Block_Process(NULL, &PlayBuff[position], DSP_BLOCK_SIZE);
		// BSP_LED_Off(LED1);

		/*if( UpdatePointer != -1 ){
			// new callback interrupt occured during dsp_block
			Debug_USART_printf("dsp out of time!\n\r");
		}*/
	}
}

// HW Enable
void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai)
{
	GPIO_InitTypeDef	GPIO_Init;

	// Enable SAI1 clock
	__HAL_RCC_SAI1_CLK_ENABLE();

	// Configure GPIOs used for SAI2
	AUDIO_SAIx_MCLK_ENABLE();
	AUDIO_SAIx_SCK_ENABLE();
	AUDIO_SAIx_FS_ENABLE();
	AUDIO_SAIx_SD_ENABLE();

	GPIO_Init.Mode	= GPIO_MODE_AF_PP;
	GPIO_Init.Pull	= GPIO_NOPULL;
	GPIO_Init.Speed	= GPIO_SPEED_FREQ_VERY_HIGH;

	GPIO_Init.Alternate	= AUDIO_SAIx_FS_AF;
	GPIO_Init.Pin			= AUDIO_SAIx_FS_PIN;
	HAL_GPIO_Init(AUDIO_SAIx_FS_GPIO_PORT, &GPIO_Init);
	GPIO_Init.Alternate	= AUDIO_SAIx_SCK_AF;
	GPIO_Init.Pin			= AUDIO_SAIx_SCK_PIN;
	HAL_GPIO_Init(AUDIO_SAIx_SCK_GPIO_PORT, &GPIO_Init);
	GPIO_Init.Alternate	= AUDIO_SAIx_SD_AF;
	GPIO_Init.Pin			= AUDIO_SAIx_SD_PIN;
	HAL_GPIO_Init(AUDIO_SAIx_SD_GPIO_PORT, &GPIO_Init);
	GPIO_Init.Alternate	= AUDIO_SAIx_MCLK_AF;
	GPIO_Init.Pin			= AUDIO_SAIx_MCLK_PIN;
	HAL_GPIO_Init(AUDIO_SAIx_MCLK_GPIO_PORT, &GPIO_Init);

	// Configure DMA used for SAI2
	__HAL_RCC_DMA2_CLK_ENABLE();

	if( hsai->Instance == AUDIO_SAIx ){
		hSaiDma.Init.Channel				= DMA_CHANNEL_10;
		hSaiDma.Init.Direction				= DMA_MEMORY_TO_PERIPH;
		hSaiDma.Init.PeriphInc				= DMA_PINC_DISABLE;
		hSaiDma.Init.MemInc					= DMA_MINC_ENABLE;
		hSaiDma.Init.PeriphDataAlignment	= DMA_PDATAALIGN_HALFWORD;
		hSaiDma.Init.MemDataAlignment		= DMA_MDATAALIGN_HALFWORD;
		hSaiDma.Init.Mode					= DMA_CIRCULAR;
		hSaiDma.Init.Priority				= DMA_PRIORITY_HIGH;
		hSaiDma.Init.FIFOMode				= DMA_FIFOMODE_ENABLE;
		hSaiDma.Init.FIFOThreshold			= DMA_FIFO_THRESHOLD_FULL;
		hSaiDma.Init.MemBurst				= DMA_MBURST_SINGLE;
		hSaiDma.Init.PeriphBurst			= DMA_PBURST_SINGLE;

		// Select the DMA instance to be used for the transfer : DMA2_Stream6
		hSaiDma.Instance                 = DMA2_Stream6;

		// Associate the DMA handle
		__HAL_LINKDMA(hsai, hdmatx, hSaiDma);

		// Deinitialize the Stream for new transfer
		HAL_DMA_DeInit(&hSaiDma);

		// Configure the DMA Stream
		if( HAL_OK != HAL_DMA_Init(&hSaiDma) ){
			Debug_USART_printf("dma failed to init");
		}
	}

	HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 0x01, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
}

// Sets a callback flag to process a block of audio in main loop
// Alternatively, could call DSP_Block_Process() inside the interrupt?
	// traditionally i've done the irq approach
	// prioritizes audio processing rather than dumping in the stack
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
	UpdatePointer = PLAY_HALF_BUFF;
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
	UpdatePointer = 0;
}

void DMA2_Stream6_IRQHandler(void)
{
	HAL_DMA_IRQHandler(SaiHandle.hdmatx);
}