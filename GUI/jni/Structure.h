/*
 * Structure.h
 *
 *  Created on: 2014-4-11
 *      Author: Administrator
 */

#ifndef STRUCTURE_H_
#define STRUCTURE_H_

#include "Timer.h"
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
	double Tqb;
	int NoVoiceCount;
	int VADflag;
	Timer* timer;
	float* inputBuffer;
	float* outputBuffer;
	float* xl;
	float* xh;
	float* Dsbuf;
	float* wts;
	float* window;
	float* FFTbuffer;
	float* dctm;
	float* mfcc;
	float Debug[10];
	Transform* transform;
} Variables;

#endif /* STRUCTURE_H_ */
