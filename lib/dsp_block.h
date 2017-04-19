#ifndef _dsp_block_
#define _dsp_block_

#include <stm32f7xx.h>

// public declarations
void DSP_Sample_Rate( uint32_t sample_rate );
void DSP_Block_Process( uint16_t* in, uint16_t* out, uint16_t b_size );

#endif