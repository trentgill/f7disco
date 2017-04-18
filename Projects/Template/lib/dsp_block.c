#include "dsp_block.h"

// private variables
uint32_t s_rate = 48000; // default

// private func prototypes
void codec_to_array(uint16_t* codec, uint16_t* ch0, uint16_t* ch1, uint16_t b_size);
void array_to_codec(uint16_t* ch0, uint16_t* ch1, uint16_t* codec, uint16_t b_size);

// public functions
void DSP_Sample_Rate( uint32_t sample_rate )
{
	s_rate = sample_rate;
}

void DSP_Block_Process(uint16_t* in_codec, uint16_t* out_codec, uint16_t b_size)
{
	uint16_t in[2][b_size];
	uint16_t out[2][b_size];
	uint16_t i, j;
	static uint16_t pitch[2] = {0,0};
	
	codec_to_array(in_codec, in[0], in[1], b_size);

	// process 1 sample at a time
	for( i=0; i<b_size; i++ ){
		// single sample process
		out[0][i] = pitch[0];
		out[1][i] = pitch[1];
		pitch[0] += 300;
		pitch[1] += 400;
	}

	array_to_codec(out[0], out[1], out_codec, b_size);
}

// private funcs
void codec_to_array(uint16_t* codec, uint16_t* ch0, uint16_t* ch1, uint16_t b_size)
{
	for( uint16_t i=0; i<b_size; i++ ){
		*ch0++ = *codec++;
		*ch1++ = *codec++;
	}
}

void array_to_codec(uint16_t* ch0, uint16_t* ch1, uint16_t* codec, uint16_t b_size)
{
	for( uint16_t i=0; i<b_size; i++ ){
		*codec++ = *ch0++;
		*codec++ = *ch1++;
	}
}