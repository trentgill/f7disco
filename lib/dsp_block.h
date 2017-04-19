#ifndef _dsp_block_
#define _dsp_block_

#include <stm32f7xx.h>

#define MAX16B	0x7FFF
#define iMAX16B	(1/ (float)MAX16B)

// public declarations
void DSP_Sample_Rate( uint32_t sample_rate, uint16_t b_size );
void DSP_Block_Process( uint16_t* in, uint16_t* out, uint16_t b_size );

#endif