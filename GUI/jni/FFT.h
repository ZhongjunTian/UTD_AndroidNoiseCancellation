#ifndef TRANSFORMS_H
#define TRANSFORMS_H

#include "SpeechProcessing.h"

Transform* newTransform(int points);
void transformMagnitude(Transform* transform, float* output);
void destroyTransform(Transform** transform);

void DFT(Transform* dft, float* input);
void FFT(Transform* fft);
void IFFT(Transform* fft);

#endif
