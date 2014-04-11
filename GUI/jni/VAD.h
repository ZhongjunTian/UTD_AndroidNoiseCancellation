#ifndef VAD_H_
#define VAD_H_

#include "Structure.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
void buble_Sort(float* a,int length);
void quick_Sort(int a[],int left,int right);
void down_sample_filter_low_pass(float* input,float* output,int windowsize);
void down_sample_filter_high_pass(float* input,float* output,int windowsize);
int VAD(float* x, Variables* P);

#endif /* VAD_H_ */
