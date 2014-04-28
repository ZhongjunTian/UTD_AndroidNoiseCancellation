/*
 ============================================================================
 Name        : VAD.c
 Author      : ZhongJun
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "VAD.h"

//extern struct Variables;

void buble_Sort(float* a,int length)
{
	int i,j;
	float k;
/*	for(j=0;j<length;j++)
	{
		for(i=0;i<j;i++)
		{
			if(a[i]>a[i+1])
			{
				k=a[i];
				a[i]=a[i+1];
				a[i+1]=k;
			}
		}
	}
	*/
	for (i = 0; i < length; i++) {
	        for (j = length - 1; j > i; j--) {
	            if (a[j] < a[j-1]) {
	                k = a[j-1];
	                a[j-1] =  a[j];
	                a[j] = k;
	            }
	        }
	}
}

void quick_Sort(int a[],int left,int right)
{
   int i,j,temp;
   i=left;
   j=right;
   temp=a[left];
   if(left>right)
      return;
   while(i!=j)/*find the final position*/
   {
      while(a[j]>=temp && j>i)
         j--;
      if(j>i)
         a[i++]=a[j];
       while(a[i]<=temp && j>i)
          i++;
       if(j>i)
          a[j--]=a[i];

   }
   a[i]=temp;
   quick_Sort(a,left,i-1);/*recursion left*/
   quick_Sort(a,i+1,right);/*recursion right*/
}

void down_sample_filter_low_pass(float* input,float* output,int windowsize)
{
	float coeffs[12]={0.0154041093270274,0.00349071208421747,-0.117990111148191,-0.0483117425856330,0.491055941926747,0.787641141030194,0.337929421727622,-0.0726375227864625,-0.0210602925123006,0.0447249017706658,0.00176771186424280,-0.00780070832503415};
	unsigned int i=0;
	do {
		float temp=0;
		int j;
		for(j=0; j<12; j++){
			temp += input[i - j]*coeffs[j];
			}
		output[i/2] = temp;
		i=i+2;
	} while(i<windowsize);
}


void down_sample_filter_high_pass(float* input,float* output,int windowsize)
{
	float coeffs[12]={0.00780070832503415,0.00176771186424280,-0.0447249017706658,-0.0210602925123006,0.0726375227864625,0.337929421727622,-0.787641141030194,0.491055941926747,0.0483117425856330,-0.117990111148191,-0.00349071208421747,0.0154041093270274};
	unsigned int i=0;
	do {
		float temp=0;
		int j;
		for(j=0; j<12; j++){
			temp += input[i - j]*coeffs[j];
			}
		output[i/2] = temp;
		i=i+2;
	} while(i<windowsize);
}


short check_Range(float value)
{
	value *= 32768;
	if (value > 32767){
		return 32767;
	} else if (value < -32768) {
		return -32768;
	} else {
		return (short)value;
	}
}


int VAD(float* x, Variables* P)
{
	float* xl=P->xl;
	float* xh=P->xh;
	int length=P->stepSize;
    down_sample_filter_low_pass(x,xl,length);
    down_sample_filter_high_pass(x,xh,length);
	float suml=0;
	float sumh=0;
	float sum=0;
	int i;

	//// get sum here ////
	for(i=0; i< (length/2) ;i++){
		suml+=xl[i]*xl[i];
		sumh+=xh[i]*xh[i];
	}
	for(i=0; i< (length) ;i++){
		sum+=x[i]*x[i];
	}
	//P->Debug[0]=sum;
	//P->Debug[1]=suml;
	//P->Debug[2]=sumh;

	float D=fabsf(suml-sumh)/(length/2);
	//P->Debug[3]=D;
	float Px=sum;
	float Dw=D*(0.5+(16/log10f(2.0f))*log10f(1.0f+2*sum));
	//P->Debug[4]=Dw;
	float temp=(float)(expf(-2*Dw));
	//P->Debug[5]=temp;
	float Dc = (1-temp)/(1+temp);
	//P->Debug[6]=Dc;
	P->Ds=Dc+P->Ds*0.65;
	//// shift Ds Buffer ////
	float* Dsbuf=P->Dsbuf;
	int Dslength=P->Dslength;
	float* Dsbuf_temp=Dsbuf++;//Dsbuf_temp+1 = Dsbuf
	for(i=0;i<Dslength-1;i++){
		*Dsbuf_temp=*Dsbuf;
		Dsbuf_temp++;
		Dsbuf++;
	}
	*Dsbuf_temp= P->Ds; //Dsbuf_temp points to the latest one

	//// sort buffer ////
	float* Dsbufsort= (float*) calloc(Dslength,sizeof(float)); //we can use malloc to save time?
	Dsbuf=P->Dsbuf;
	for(i=0;i<Dslength;i++){
		Dsbufsort[i]=Dsbuf[i];
	}
	buble_Sort(Dsbufsort,Dslength);

	//// VAD ////
	i=4;
	while(((Dsbufsort[i]-Dsbufsort[i-4])<0.001) && (i<Dslength-1))
		i++;
	//P->Debug[7]=(float)i;
	float Tqb=Dsbufsort[i];

	//P->Debug[8]=Tqb;
	P->Tqb = 0.975*P->Tqb + 0.025*(double)Tqb;
	int VADDec;
    if( P->Ds > P->Tqb ){
        P->NoVoiceCount = 0;
        P->VADflag = 0;
        VADDec = 1;
        P->VADtemp=1;
    }
    else{
        P->VADtemp=0;
        if(P->VADflag)
            VADDec = 0;
        else{
            VADDec = 1;
            P->NoVoiceCount += 1;
            //P->Debug[1]=(int)(Dslength/5);
            if(P->NoVoiceCount>(int)(Dslength/40))
                P->VADflag = 1;
			}
    }
    //P->Debug[9]=VADDec;
    //if (100<P->Debug[0] && P->Debug[0]<250){
    //	__android_log_print(ANDROID_LOG_ERROR, "debug", "No.%3.0f  Ds %f, P->Tqb %f, Tqb %f,i=%u ,Dslength %f",P->Debug[0], P->Ds , P->Tqb, Tqb,i,P->Debug[1]);
    //}
	free(Dsbufsort);
    return(VADDec);
}
