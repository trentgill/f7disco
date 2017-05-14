#pragma once

#include <stm32f7xx.h>

#define MAX16B	0x7FFF
#define iMAX16B	(1/ (float)MAX16B)

#define VOICE_COUNT 4
#define BASE_PITCH  0.004
#define MOD_BASE    0.000007

// public declarations
void DSP_Block_Init( uint32_t sample_rate, uint16_t b_size );
void DSP_Block_Process( uint16_t* in, uint16_t* out, uint16_t b_size );
