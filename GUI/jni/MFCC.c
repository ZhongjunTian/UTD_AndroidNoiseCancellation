#include "MFCC.h"

void MFCC(Variables* P)
{

}

//*********part 1 FFT
void filter(float* x,int length)
{
	int i;
	float cur_x=*x;
	for(i=length-1;i>0;i--){
		float pre_x=cur_x;
		x++;
		cur_x=*x;
		*x=cur_x-0.97*pre_x;
	}
}

void powspec_specgram(Variables* P)
{
	Transform* T=P->transform;
	float* x=P->inputBuffer;

	float* window = (float*) malloc(P->windowSize*sizeof(float));
	hanning(window,P->windowSize);

	int i;
	for(i=0;i<P->windowSize;i++){
		T->real[i]=x[i]*32768.0f*window[i];
	}
	for(i=P->windowSize;i<T->points;i++){
		T->real[i]=0;
	}
	FFT(T); // 取一半+1个数据
	//结果取一半+1个数据   并且 .^2
	//float e = sum();

}

void hanning(float* window,int length)
{
	const float _2pi=6.283185307179586476925286766559;
	int i;
	float N=(float)length+1;
	for(i=1;i<=length;i++){
		*window=0.5-(float)cos(_2pi*i/N)/2;
		window++;
	}
}

//********part 2 freq. domain , triangle filter

#define NFFT 40
void fft2melmx(Variables* P)
{
	float wts[65][40];
	float bingraqs[42];
	int i;
	float min=0;
	float max=hz2mel(4000,0);
	for(i=0;i<42;i++){
		bingraqs[i]=mel2hz((i)*(max-min)/41,0);
	}
}

float hz2mel(float f,int htk)
{
	//  Optional htk = 1 uses the mel axis defined in the HTKBook
	//  otherwise use Slaney's formula

	float  z;

	if(htk == 1)
	//  Optional htk = 1 uses the mel axis defined in the HTKBook
	  z = 2595 * log10(1+f/700);

	else{
	 // Mel fn to match Slaney's Auditory Toolbox mfcc.m
		float f_0 = 0; // 133.33333;
		float  f_sp = 66.666666666666666f; // 66.66667;
		float  brkfrq = 1000.0f;
		float  brkpt  = (brkfrq - f_0)/f_sp;  // ==15 starting mel value for log region

		if(f < 1000.0f)
			z = (f - f_0)/f_sp;
		else{
			float  logstep = 1.071170287494468f; // //he magic 1.0711703 which is the ratio needed to get from 1000 Hz to 6400 Hz in 27 steps, and is *almost* the ratio between 1000 Hz and the preceding linear filter center at 933.33333 Hz (actually 1000/933.33333 = 1.07142857142857 and  exp(log(6.4)/27) = 1.07117028749447)
			z = brkpt+(log(f/brkfrq))/log(1.071170287494468f);
		}
	}
	return(z);
}

float mel2hz(float z,int htk)
{
	float f;
	if(htk == 1)
		f = 700*(exp((z/2595)-1)*0.43429448190325182765f);//700*(10.^(z/2595)-1)  10^2=exp(x*ln10)
	else{

		float f_0 = 0; // 133.33333;
		float f_sp = 66.666666666666666f; // 66.66667;
		float brkfrq = 1000.0f;
		float brkpt  = (brkfrq - f_0)/f_sp;  // starting mel value for log region
		float logstep = 1.071170287494468f; // the magic 1.0711703 which is the ratio needed to get from 1000 Hz to 6400 Hz in 27 steps, and is *almost* the ratio between 1000 Hz and the preceding linear filter center at 933.33333 Hz (actually 1000/933.33333 = 1.07142857142857 and  exp(log(6.4)/27) = 1.07117028749447)

		if(z < brkpt)
			f = f_0 + f_sp*z;
		else
			f = brkfrq*exp(log(logstep)*(z-brkpt));
	}
	return(f);
}

