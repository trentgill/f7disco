#include "dsp_block.h"

static uint16_t pitch[2] = {0,0};

void DSP_Block_Process(uint16_t* in_codec, uint16_t* out_codec, uint16_t b_size)
{
	uint16_t in[2][b_size];
	uint16_t out[2][b_size];
	uint16_t i, j;
	
	// process 1 sample at a time
	for( i=0; i<b_size; i++ ){
		// deinterleave codec stream into u16 arrays
		in[0][i] = *in_codec++;
		in[1][i] = *in_codec++;

		// single sample process
		out[0][i] = pitch[0];
		out[1][i] = pitch[1];
		pitch[0] += 300;
		pitch[1] += 400;

		// interleave output arrays into serial codec stream
		*out_codec++ = out[0][i];
		*out_codec++ = out[1][i];
	}
}