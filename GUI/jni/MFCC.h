/*
 * MFCC.h
 *
 *  Created on: 2014Äê4ÔÂ3ÈÕ
 *      Author: Administrator
 */

#ifndef MFCC_H_
#define MFCC_H_

#include "SpeechProcessing.h"

void MFCC(Variables* P);
void filter(float* x,int length);
void powspec(Variables* P);
void hanning(float* window,int length);
void fft2melmx(Variables* P);
float hz2mel(float f,int htk);
float mel2hz(float z,int htk);

#endif /* MFCC_H_ */

