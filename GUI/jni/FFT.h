#ifndef TRANSFORMS_H
#define TRANSFORMS_H

#include "Structure.h"
#include <stdio.h>
#define M_PI 3.14159265358979323846
Transform* newTransform(int points);
void transformMagnitude(Transform* transform, float* output);
void destroyTransform(Transform** transform);

void DFT(Transform* dft, float* input);
void FFT(Transform* fft);
void IFFT(Transform* fft);

#endif
