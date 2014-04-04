#include "FFT.h"

Transform* newTransform(int points)
{
	Transform* newTransform = (Transform*)malloc(sizeof(Transform));

	newTransform->points = exp2(ceil(log(points)/log(2)));
	newTransform->real = (float*)calloc(newTransform->points,sizeof(float));
	newTransform->imaginary = (float*)calloc(newTransform->points,sizeof(float));
	newTransform->sine = (float*)malloc((newTransform->points)*sizeof(float));
	newTransform->cosine = (float*)malloc((newTransform->points)*sizeof(float));


	//precompute twiddle factors
	float arg;
	int i;
	for (i=0;i<newTransform->points/2;i++)
	{
		arg = -2*M_PI*i/newTransform->points;
		newTransform->cosine[i] = cos(arg);
		newTransform->sine[i] = sin(arg);
	}

	return newTransform;
}

void DFT(Transform* dft, float* input)
{
	int i, j;
	float arg, wI, wR;

	for(i=0;i<(dft->points);i++) {
		dft->real[i] = input[i];
		dft->imaginary[i] = 0;
	}

	for (i=0; i<(dft->points); i++) {
		//re-use twiddle factor space
		dft->sine[i] = 0.0f;
		dft->cosine[i] = 0.0f;

		for(j=0; j<(dft->points); j++) {
			arg = 2*M_PI*i*j/(dft->points);
			wR = cos(arg);
			wI = sin(arg);
			dft->sine[i] += dft->real[j] * wR + dft->imaginary[j] * wI;
			dft->cosine[i] += dft->imaginary[j] * wR - dft->real[j] * wI;

		}
	}

	for (i=0; i<(dft->points); i++) {
		dft->real[i] = dft->sine[i];
		dft->imaginary[i] = dft->cosine[i];
	}
}


void FFT(Transform* fft)
{
	int i,j,k,L,m,n,o,p,q,r;
	float tempReal,tempImaginary,cos,sin,xt,yt;
	k = fft->points;

	j=0;
	m=k/2;

	//bit reversal
	for(i=1;i<(k-1);i++)
	{
		L=m;

		while(j>=L)
		{
			j=j-L;
			L=L/2;
		}

		j=j+L;

		if(i<j)
		{
			tempReal=fft->real[i];
			tempImaginary=fft->imaginary[i];
			fft->real[i]=fft->real[j];
			fft->imaginary[i]=fft->imaginary[j];
			fft->real[j]=tempReal;
			fft->imaginary[j]=tempImaginary;
		}
	}

	L=0;
	m=1;
	n=k/2;

	//computation
	for(i=k;i>1;i=(i>>1))
	{
		L=m;
		m=2*m;
		o=0;

		for(j=0;j<L;j++)
		{
			cos=fft->cosine[o];
			sin=fft->sine[o];
			o=o+n;

			for(p=j;p<k;p=p+m)
			{
				q=p+L;

				xt=cos*fft->real[q]-sin*fft->imaginary[q];
				yt=sin*fft->real[q]+cos*fft->imaginary[q];
				fft->real[q]=(fft->real[p]-xt);
				fft->imaginary[q]=(fft->imaginary[p]-yt);
				fft->real[p]=(fft->real[p]+xt);
				fft->imaginary[p]=(fft->imaginary[p]+yt);
			}
		}
		n=n>>1;
	}
}

void IFFT(Transform* fft)
{
	int i,j,k,L,m,n,o,p,q,r;
	float tempReal,tempImaginary,cos,sin,xt,yt;
	k = fft->points;

	/*for(i=0;i<k;i++)
	{
		fft->real[i] = fft->real[i];
		fft->imaginary[i] = fft->imaginary[i];
	} */

	j=0;
	m=k/2;

	//bit reversal
	for(i=1;i<(k-1);i++)
	{
		L=m;

		while(j>=L)
		{
			j=j-L;
			L=L/2;
		}

		j=j+L;

		if(i<j)
		{
			tempReal=fft->real[i];
			tempImaginary=fft->imaginary[i];
			fft->real[i]=fft->real[j];
			fft->imaginary[i]=fft->imaginary[j];
			fft->real[j]=tempReal;
			fft->imaginary[j]=tempImaginary;
		}
	}

	L=0;
	m=1;
	n=k/2;

	//computation
	for(i=k;i>1;i=(i>>1))
	{
		L=m;
		m=2*m;
		o=0;

		for(j=0;j<L;j++)
		{
			cos=fft->cosine[o];
			sin=fft->sine[o]*-1;
			o=o+n;

			for(p=j;p<k;p=p+m)
			{
				q=p+L;

				xt=cos*fft->real[q]-sin*fft->imaginary[q];
				yt=sin*fft->real[q]+cos*fft->imaginary[q];
				fft->real[q]=(fft->real[p]-xt);
				fft->imaginary[q]=(fft->imaginary[p]-yt);
				fft->real[p]=(fft->real[p]+xt);
				fft->imaginary[p]=(fft->imaginary[p]+yt);
			}
		}
		n=n>>1;
	}

	for(i=0;i<k;i++)
	{
		fft->real[i] /= k;
		fft->imaginary[i] /= k;
	}
}

void transformMagnitude(Transform* transform, float* output)
{
	int n;
	for (n=0; n<transform->points; n++)
	{
		output[n] = sqrt(transform->real[n]*transform->real[n]+transform->imaginary[n]*transform->imaginary[n]);
	}
}

void destroyTransform(Transform** transform)
{
	if(*transform != NULL){
		if((*transform)->cosine != NULL){
			free((*transform)->cosine);
			(*transform)->cosine = NULL;
		}
		if((*transform)->sine != NULL){
			free((*transform)->sine);
			(*transform)->sine = NULL;
		}
		if((*transform)->real != NULL){
			free((*transform)->real);
			(*transform)->real = NULL;
		}
		if((*transform)->imaginary != NULL){
			free((*transform)->imaginary);
			(*transform)->imaginary = NULL;
		}
		free(*transform);
		*transform = NULL;
	}
}
