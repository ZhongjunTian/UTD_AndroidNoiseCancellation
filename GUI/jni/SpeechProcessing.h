

#ifndef SPEECH_H
#define SPEECH_H
#include "Timer.h"

typedef struct Transform {
		int points;
		float* sine;
		float* cosine;
		float* real;
		float* imaginary;
		//void (*doTransform)(struct Transform* transform, float* input);
} Transform;

typedef struct LogMMSE {
	float* noise_mean;
	float* noise_mu2;
	float* sig2;
	float* Xk_prev;
	float* Rk_prev;
	int NumWinPmin;
	float** SNP;
	float* SPP;
	float* hw;
	float* x_old;
} LogMMSE;


typedef struct {
	int frequency;
	int stepSize;
	int windowSize;
	int overlap;
	Timer* timer;
	float* inputBuffer;
	short* originalInput;
	float* outputBuffer;
	//**VAD**//
	float* xl;
	float* xh;
	int Dslength;
	float Ds;
	float* Dsbuf;
	double Tqb;
	int NoVoiceCount;
	int VADflag;
	int VADtemp;
	//**MFCC**//
	float* wts;
	float* window;
	float* FFTbuffer;
	float* dctm;
	float* mfcc;
	float* mfcc_aspectrum_40;
	float* mfcc_pspectrum;
	float Debug[10];
	Transform* transform;
	LogMMSE* logMMSE;
	int GMMClass;
	int* decisionBuffer;
	int decisionBufferLength;
	int decisionBufferCounter;
	int decide[4];
} Variables;
#endif

