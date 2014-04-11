/*
 * MFCC.h
 *
 *  Created on: 2014Äê4ÔÂ3ÈÕ
 *      Author: Administrator
 */

#ifndef MFCC_H_
#define MFCC_H_

#include "Structure.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//#include "FFT.h"

void MFCC(Variables* P);
void filter(float* x,int length);
void powspec(Variables* P);
void hanning(float* window,int length);
void fft2melmx(Variables* P);
void spec2cep(Variables* P);
float hz2mel(float f,int htk);
float mel2hz(float z,int htk);

#define M_PI 3.14159265358979323846
Transform* newTransform(int points);
void transformMagnitude(Transform* transform, float* output);
void destroyTransform(Transform** transform);

void DFT(Transform* dft, float* input);
void FFT(Transform* fft);
void IFFT(Transform* fft);

#endif /* MFCC_H_ */

