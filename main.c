/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-April-2016
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
  
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "lib/debug_usart.h"
#include "lib/disco_hw.h"
#include "lib/disco_term.h"
#include "lib/disco_codec.h"

#include "usb/usb_keyboard.h"

extern uint32_t uwTick;  // time since reboot (ms)

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern USBH_HandleTypeDef hUSBHost;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void MPU_Config(void);
static void CPU_CACHE_Enable(void);
void oncePerSecond(void);

static void HID_InitApplication(void);


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
__IO float master_pitch;
__IO float master_mod;
extern uint8_t DSP_Dirty;

// const float m_pitches[4] = {1, 5/3, 10/9, 3/2};
const float m_pitches[4] = {1, 1.66667, 1.11111, 1.5};
const float m_modrates[5] = {1,0.72,0.3,1.3,1.7};

int main(void)
{
	// Configure low-level
	MPU_Config();
	CPU_CACHE_Enable();
	HAL_Init();
	SystemClock_Config();

	// HW initialization
	Debug_USART_Init();
	Disco_HW_Init();
	Disco_Codec_Init();

	// USB stack
	BSP_IO_Init(); // Init IO Expander
	HID_MenuInit(); // Init Menu?!
	USBH_Init(&hUSBHost, USBH_UserProcess, 0); // Init Host Lib
	USBH_RegisterClass(&hUSBHost, USBH_HID_CLASS);
	USBH_Start(&hUSBHost);

	// APPLICATION CODE
	HAL_Delay(100);
	Debug_USART_printf("time to party!\n\r");

	oncePerSecond(); // call without waiting

	// Infinite loop
	static uint32_t lastEventTime = 0;
	static uint8_t lastButton;
	static uint8_t p_count, m_count;
	char state[4] = { '0', ' ', '0', '\0' };

	while(1){
		// Disco_Codec_Loop(); // process audio loop
		if( (uwTick - lastEventTime) > 1000 ){
			// once per second
			lastEventTime = uwTick;
			oncePerSecond();
		} else if( uwTick < lastEventTime ){
			// OVERFLOW
			lastEventTime = uwTick;
		}

		USBH_Process(&hUSBHost); // USB Host BG task
		HID_MenuProcess(); // HID Menu Process

		// hacked synthesis settings change
		/*uint8_t press = BSP_PB_GetState(0);
		if( (lastButton != press) && (press == 1) ){
			master_pitch   = m_pitches[p_count++];
			master_mod     = m_modrates[m_count++];
			DSP_Dirty      = 1;
			if(p_count >= 4){ p_count = 0; }
			if(m_count >= 5){ m_count = 0; }
			state[0] = p_count+48;
			state[2] = m_count+48;
			Disco_Term_Read_Debug(state);
	    }
	    lastButton = press;*/
	}
	return 0;
}

uint32_t inker = 0;
void oncePerSecond(void)
{
	static uint8_t flip;
	unsigned char strung[51] = " \0";
	Disco_HW_Loop();
	
	// HAL_Delay(100);
	if(!flip){
		// READ
		sprintf(strung, "%d", inker); // convert inker to string
	    strcat(strung, " + 1\0");
	    Disco_Term_Read_String(strung);
	} else {
		// EVAL
		strcpy( strung, Disco_Term_Eval() );
		inker = atoi(strung);
    
		// debug
		strcat( strung, "\n\r" );
		Debug_USART_printf( strung );
	}
	flip ^= 1;
}


// LOW LEVEL SYS INIT
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 216000000 -> only 200 RN!
  *            HCLK(Hz)                       = 216000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 432
  *            PLL_P                          = 2
  *            PLL_Q                          = 9
  *            PLL_R                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 7
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	HAL_StatusTypeDef ret = HAL_OK;
	
	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();
	
	/* The voltage scaling allows optimizing the power consumption when the device is 
		clocked below the maximum system frequency, to update the voltage scaling value 
		regarding system frequency refer to product datasheet.  */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 400;  // for 216MHz use 432 !!!!!!!!!!
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;
	RCC_OscInitStruct.PLL.PLLR = 7;  

	ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if(ret != HAL_OK) {
		Error_Handler();
	}

	/* Activate the OverDrive to reach the 216 MHz Frequency */	
	ret = HAL_PWREx_EnableOverDrive();
	if(ret != HAL_OK) {
		Error_Handler();
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
	if(ret != HAL_OK) {
		Error_Handler();
	}
}

static void Error_Handler(void)
{
	while(1) {
		;;
	}
}

static void MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct;
	
	// Disable the MPU
	HAL_MPU_Disable();

	// Configure the MPU attributes as WT for SRAM
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = 0x20020000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	// Enable the MPU
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

static void CPU_CACHE_Enable(void)
{
	// Enable I-Cache
	SCB_EnableICache();

	// Enable D-Cache
	SCB_EnableDCache();
}

#ifdef  USE_FULL_ASSERT

// Reports the name of the source file and the source line number
void assert_failed(uint8_t* file, uint32_t line)
{ 
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	// Infinite loop
	while (1) {
		;;
	}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
