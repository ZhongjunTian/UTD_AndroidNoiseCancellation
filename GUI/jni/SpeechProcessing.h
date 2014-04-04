
#include "Timer.h"
#include "VAD.h"
#include "FFT.h"
#include "MFCC.h"
#include "FFT.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef STRUCT_VARIABLES
#define STRUCT_VARIABLES

typedef struct Transform {
		int points;
		float* sine;
		float* cosine;
		float* real;
		float* imaginary;
		//void (*doTransform)(struct Transform* transform, float* input);
} Transform;

typedef struct {
	int frequency;
	int stepSize;
	int windowSize;
	int overlap;
	int Dslength;
	float Ds;
	float Tqb;
	int NoVoiceCount;
	int VADflag;
	Timer* timer;
	float* inputBuffer;
	float* outputBuffer;
	float* xl;
	float* xh;
	float* Dsbuf;
	float Debug[10];
	Transform* transform;
} Variables;

#endif

