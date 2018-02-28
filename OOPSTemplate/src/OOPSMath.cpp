/*
  ==============================================================================

    OOPSMath.c
    Created: 22 Jan 2017 7:02:56pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#if _WIN32 || _WIN64

#include "OOPSMath.h"
#include "OOPSWavetables.h"

#else

#include "OOPSMath.h"
#include "OOPSWavetables.h"

#endif

// The C-embedded Audio Library.
#define TWO_TO_16 65536.f

#define EXPONENTIAL_TABLE_SIZE 65536

// dope af
float OOPS_chebyshevT(float in, int n){
	if (n == 0) return 1;
	else if (n == 1) return in;
	else return 2.0f * in * OOPS_chebyshevT(in, n-1) - OOPS_chebyshevT(in, n-2);
}

#if !(_WIN32 || _WIN64)
float OOPS_CompoundChebyshevT(float in, int n, float* amps){
	float T[n+1];
	T[0] = 1.0f;
	T[1] = in;
	for (int i = 2; i <= n; ++i)
		T[i] = 2*in*T[i-1] - T[i-2];
	float out = 0;
	float amp = 0;
	for (int i = 0; i < n; ++i){
		out += amps[i]*T[i+1];
		amp += amps[i];
	}
	return out / amp ;
}
#endif

float OOPS_frequencyToMidi(float f)
{
	return (69.0f + 12.0f * log2(f * INV_440));
}

// Erbe shaper
float OOPS_shaper(float input, float m_drive) 
{
    float fx = input * 2.0f;    // prescale
    float w, c, xc, xc2, xc4;

    xc = OOPS_clip(-SQRT8, fx, SQRT8);
    xc2 = xc*xc;
    c = 0.5f*fx*(3.0f - (xc2));
    xc4 = xc2 * xc2;
    w = (1.0f - xc2*0.25f + xc4*0.015625f) * WSCALE;
    float shaperOut = w*(c+ 0.05f*xc2)*(m_drive + 0.75f);
    shaperOut *= 0.5f;    // post_scale
    return shaperOut;
}

float OOPS_reedTable(float input, float offset, float slope) 
{
    float output = offset + (slope * input);
    if ( output > 1.0f) output = 1.0f;
    if ( output < -1.0f) output = -1.0f;
    return output;
}

float   OOPS_softClip(float val, float thresh) 
{
	float x;
	
	if(val > thresh)
	{
			x = thresh / val;
			return (1.0f - x) * (1.0f - thresh) + thresh;
	}
	else if(val < -thresh)
	{
			x = -thresh / val;
			return -((1.0f - x) * (1.0f - thresh) + thresh);
	}
	else
	{
		return val;
	}
}

float   OOPS_clip(float min, float val, float max) 
	{
    
    if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    } else {
        return val;
    }
}

oBool     OOPS_isPrime(uint64_t number )
{
    if ( number == 2 ) return OTRUE;
    if ( number & 1 ) {
        for ( int i=3; i<(int)sqrt((double)number)+1; i+=2 )
            if ( (number % i) == 0 ) return OFALSE;
        return OTRUE; // prime
    }
    else return OFALSE; // even
}

// Adapted from MusicDSP: http://www.musicdsp.org/showone.php?id=238
float OOPS_tanh(float x)
{
    
    if( x < -3 )
        return -1;
    else if( x > 3 )
        return 1;
    else
        return x * ( 27 + x * x ) / ( 27 + 9 * x * x );
}

//-----------------------------------------------------------------------------
// name: mtof()
// desc: midi to freq, from PD source
//-----------------------------------------------------------------------------
float OOPS_midiToFrequency(float f)
{
    if( f <= -1500.0f ) return (0);
    else if( f > 1499.0f ) return (OOPS_midiToFrequency(1499.0f));
    else return ( powf(2.0f, (f - 69.0f) / 12.0f) * 440.0f );
}
