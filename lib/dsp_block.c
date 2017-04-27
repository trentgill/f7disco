#include "dsp_block.h"
#include "../../wrLib/wrLpGate.h"
#include "../../wrLib/wrFuncGen.h"
#include "../../wrLib/wrMath.h"
#include "debug_usart.h"
// #include "../../wrLib/wrOscSine.h"

// private variables
uint32_t s_rate = 48000; // default

// private structures
lpgate_t vcoGate[VOICE_COUNT];
func_gen_t vcoRamp[VOICE_COUNT];
func_gen_t envRamp[VOICE_COUNT];
// osc_sine_t vcoSine;

// private func prototypes
void codec_to_array(uint16_t* codec, float* ch0, float* ch1, uint16_t b_size);
void array_to_codec(float* ch0, float* ch1, uint16_t* codec, uint16_t b_size);

// public functions
	// NB: snuck some init functions in here :)
void DSP_Block_Init( uint32_t sample_rate, uint16_t b_size )
{
	s_rate = sample_rate;
	for( uint8_t j=0; j<VOICE_COUNT; j++ ){
		lpgate_init( &vcoGate[j], 1, 1, b_size );
		function_init( &vcoRamp[j], -1 ); // looping!
		function_ramp( &vcoRamp[j], j/VOICE_COUNT );
		vcoRamp[j].rate = 0.007 * ((float)j+1);

		function_init( &envRamp[j], -1 ); // looping!
		function_ramp( &envRamp[j], 0.5 );
		envRamp[j].rate = 0.00001 * ((float)j+2);
	}
	// osc_sine_init( &vcoSine );
}

extern float master_pitch;
extern float master_mod;
__IO uint8_t DSP_Dirty = 0;
void DSP_Block_Process(uint16_t* in_codec, uint16_t* out_codec, uint16_t b_size)
{
	float	 in[2][b_size];
	float	 osc[VOICE_COUNT][b_size];
	float    env[VOICE_COUNT][b_size];
	float	 out[2][b_size];
	float    mix[b_size];
	uint16_t i, j;

	if( DSP_Dirty ){
		// apply changes
		for( j=0; j<VOICE_COUNT; j++ ){
			vcoRamp[j].rate = BASE_PITCH * master_pitch * (j+1);
			envRamp[j].rate = MOD_BASE * master_mod * (j+2);
		}
		DSP_Dirty = 0;
	}

	codec_to_array(in_codec, in[0], in[1], b_size);

		// oscillators & gates
	for( i=0; i<b_size; i++ ){
		// level is mapped from /| to |\ at 0.3 gain
		mix[i] = 0; // clear mix
		out[0][i] = 0;
		out[1][i] = 0;
		for( j=0; j<VOICE_COUNT; j++ ){
			env[j][i] = 0.2 - 0.2*function_step( &envRamp[j] );
			osc[j][i] = function_step( &vcoRamp[j] );
			if(j%2 != 0){
				out[0][i] += lpgate_step( &vcoGate[j], env[j][i], osc[j][i] );
			} else {
				out[1][i] += lpgate_step( &vcoGate[j], env[j][i], osc[j][i] );
			}
		}

		// clip output & copy to left & right
		// out[1][i] = out[0][i] = lim_f(mix[i], -1.0, 1.0);
		out[0][i] = lim_f(out[0][i], -0.99, 0.99);
		out[1][i] = lim_f(out[1][i], -0.99, 0.99);
	}

	array_to_codec(out[0], out[1], out_codec, b_size);
}

// private funcs
void codec_to_array(uint16_t* codec, float* ch0, float* ch1, uint16_t b_size)
{
	for( uint16_t i=0; i<b_size; i++ ){
		*ch0++ = iMAX16B * (float)(int16_t)(*codec++);
		*ch1++ = iMAX16B * (float)(int16_t)(*codec++);
	}
}

void array_to_codec(float* ch0, float* ch1, uint16_t* codec, uint16_t b_size)
{
	for( uint16_t i=0; i<b_size; i++ ){
		*codec++ = (int16_t)(*ch0++ * (float)MAX16B);
		*codec++ = (int16_t)(*ch1++ * (float)MAX16B);
	}
}