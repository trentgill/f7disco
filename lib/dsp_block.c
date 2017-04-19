#include "dsp_block.h"
#include "../../wrLib/wrLpGate.h"
// #include "../../wrLib/wrOscSine.h"

// private variables
uint32_t s_rate = 48000; // default

// private structures
lpgate_t vcoGate;
// osc_sine_t vcoSine;

// private func prototypes
void codec_to_array(uint16_t* codec, uint16_t* ch0, uint16_t* ch1, uint16_t b_size);
void array_to_codec(uint16_t* ch0, uint16_t* ch1, uint16_t* codec, uint16_t b_size);

// public functions
	// NB: snuck some init functions in here :)
void DSP_Sample_Rate( uint32_t sample_rate, uint16_t b_size )
{
	s_rate = sample_rate;
	lpgate_init( &vcoGate, 1, 1, b_size );
	// osc_sine_init( &vcoSine );
}

void DSP_Block_Process(uint16_t* in_codec, uint16_t* out_codec, uint16_t b_size)
{
	uint16_t in[2][b_size];
	float	 tmp[2][b_size];
	uint16_t out[2][b_size];
	uint16_t i, j;
	static float pitch[2] = {0,0};
	
	codec_to_array(in_codec, in[0], in[1], b_size);

	// process 1 sample at a time
	for( i=0; i<b_size; i++ ){
		
		tmp[0][i] = pitch[0];
		tmp[1][i] = pitch[1];
		pitch[0] += iMAX16B * 300.0;
		pitch[1] += iMAX16B * 400.0;

		// overflow
		if(pitch[0] >= 1.0){
			pitch[0] = -1.0;
		}
		if(pitch[1] >= 1.0){
			pitch[1] = -1.0;
		}
	}

	for( i=0; i<b_size; i++ ){
		out[0][i] = (uint16_t)(MAX16B *
			lpgate_step( &vcoGate, 0.1, tmp[0][i]) );
		out[1][i] = (uint16_t)(MAX16B * tmp[1][i]);
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