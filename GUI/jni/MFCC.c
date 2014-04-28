#include "MFCC.h"

void MFCC(Variables* P)
{
	int i,j;
	int point=P->transform->points;
	int hpoint=point/2+1;

	for(i=0;i<hpoint;i++){
		*(P->mfcc_pspectrum + i)=*(P->transform->real + i)**(P->transform->real + i) + *(P->transform->imaginary + i)**(P->transform->imaginary + i);
	}//result is pspectrum, store in real[0:64]

	float aspectrum[40];
	for(i=0;i<40;i++){
		*(P->mfcc_aspectrum_40 + i)=0;
		for(j=0;j<hpoint;j++){
			if(*(P->wts+i*hpoint+j)!=0){
			*(P->mfcc_aspectrum_40 + i) += *(P->wts+i*hpoint+j)**(P->mfcc_pspectrum + j);
			}//result is aspectrum, store in imaginary[0:39]
		}
	}


	for(i=0;i<40;i++){
		*(P->mfcc_aspectrum_40 + i)=log(*(P->mfcc_aspectrum_40 + i));
	}//log(spec);
	for(i=0;i<13;i++){
		*(P->mfcc + i)=0;
		for(j=0;j<40;j++){
			*(P->mfcc + i) += *(P->dctm+40*i+j) * *(P->mfcc_aspectrum_40 + j);
		}
	}//cep = dctm*log(spec);

	const float liftwts[13]={1.0f,1.0f,1.51571656651040f,1.93318204493176f,2.29739670999407f,2.62652780440377f,2.93015605158352f,3.21409584971604f,3.48220225318450f,3.73719281884655f,  3.98107170553497f,  4.21536913309190f,  4.44128606984584};
	for(i=0;i<13;i++){
		*(P->mfcc + i) *=liftwts[i];
	}//   y = liftwts.*x; //liftwts = [1, (1+L/2*sin([1:(ncep-1)]*pi/L))];
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

/*void powspec_specgram(Variables* P)
{
	Transform* T=P->transform;
	float* x=P->inputBuffer;

	//float* window = (float*) malloc(P->windowSize*sizeof(float));
	hanning(P->window,P->windowSize);

	int i;
	for(i=0;i<P->windowSize;i++){
		T->real[i]=x[i]*32768.0f*P->window[i];
	}
	for(i=P->windowSize;i<T->points;i++){
		T->real[i]=0;
	}
	FFT(T); // 取一半+1个数据
	//结果取一半+1个数据   并且 .^2
	//float e = sum();

}*/

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
	int hpoint=P->transform->points/2 +1;
	float* fftfrqs=(float*)calloc(hpoint,sizeof(float));//*************////P->transform->points+2
	float binfrqs[42];
	int i,j;
	for(i=0;i<hpoint;i++){
		fftfrqs[i]=(float)(i*P->frequency)/P->transform->points;//*************//
	}
	float min=0;
	float max=hz2mel(P->frequency/2,0);//*************//
	for(i=0;i<42;i++){
		binfrqs[i]=mel2hz((i)*(max-min)/41.0f,0);
	}
	for(i=0;i<40;i++){
		for(j=0;j< hpoint ;j++){
			float loslope=(fftfrqs[j]-binfrqs[i])/(binfrqs[i+1]-binfrqs[i]);
			float hislope=(binfrqs[i+2]-fftfrqs[j])/(binfrqs[i+2] - binfrqs[i+1]);
			float min= loslope<hislope? loslope: hislope;
			float max=0.0f >min? 0:min;
			*(P->wts + hpoint*i +j) = max;
			if(max!=0){
				*(P->wts + hpoint*i +j) *= 2.0f/(binfrqs[i+2]-binfrqs[i]);
			}
		}
	}
	free(fftfrqs);
}

void spec2cep(Variables* P)
{
	float* dctm=P->dctm;
	int i,j;
	for(i=0;i<13;i++){
		int k=0;
		for(j=1;j< 40*2;j+=2){
			double temp=j*(3.1415926535897932384626433832795/(40.0*2.0));
			*(dctm+40*i+k)=cos(i*j*(3.1415926535897932384626433832795/(40.0*2.0)))*sqrt(2.0/40.0);
			k++;
		}
	}
	for(j=0;j< 40;j++){
		*(dctm+j)*=sqrt(0.5);
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


//////////////////////////////////////////////////////////////////////////////////////////////


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

