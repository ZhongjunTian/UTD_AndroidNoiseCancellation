#include <jni.h>
#include "SpeechProcessing.h"
#include "VAD.h"
#include "LogMMSE.h"
#include "MFCC.h"
#include "GMM.h"
#include <math.h>
#include <stdio.h>


static void
compute(JNIEnv *env, jobject thiz,  jlong memoryPointer, jshortArray input)
{
	Variables* inParam = (Variables*) memoryPointer;
	startTimer(inParam->timer);
	
	short *_in = (*env)->GetShortArrayElements(env, input, NULL);
	
	int i, overlap, stepsize;
	overlap = inParam->overlap;
	stepsize = inParam->stepSize;

	for(i=0; i<overlap; i++)
	{
		inParam->inputBuffer[i] = inParam->inputBuffer[stepsize + i];
	}

	for (i=0; i<stepsize; i++)
	{
		inParam->originalInput[i] = _in[i];
		inParam->inputBuffer[overlap + i] = _in[i]/32768.0f;
	}

	(*env)->ReleaseShortArrayElements(env, input, _in, 0);
	//Processing logic here. For now this placeholder will copy the input
	//to the output array for the processing function and scale it by 1/2.
	//For switching between suppressed and original output a GUI option
	//can be used to select the output channel.

	inParam->Debug[0]++;//be used as a counter

	//**FFT**//
	for(i=0;i<inParam->windowSize;i++){
		*(inParam->transform->real + i) = *(inParam->inputBuffer + i) * (*(inParam->window+i)); /// moved out *32768.0f  ************
		*(inParam->transform->imaginary + i) =0;
	}
	for(i=inParam->windowSize;i< inParam->transform->points;i++){
		*(inParam->transform->real + i) = 0;
		*(inParam->transform->imaginary + i) =0;
	}
	FFT(inParam->transform);

	//**VAD**//
	int VADDec = VAD(inParam->inputBuffer+overlap, inParam);

	//**MFCC and GMM**//
	if(inParam->VADtemp==0){
		MFCC(inParam);
		inParam->GMMClass=GMM(inParam);

		inParam->decisionBufferCounter++;
		if(inParam->decisionBufferCounter>=inParam->decisionBufferLength)
			inParam->decisionBufferCounter=0;
		int ID=inParam->decisionBufferCounter;

		int new_cl=inParam->GMMClass;
		int old_cl=inParam->decisionBuffer[ID];
		inParam->decide[old_cl]--;
		inParam->decide[new_cl]++;
		inParam->decisionBuffer[ID]=new_cl;

		if(inParam->decide[1]>inParam->decide[2] &&inParam->decide[1]>inParam->decide[3] )
			inParam->GMMClass=1;
		else if(inParam->decide[2]>inParam->decide[1] &&inParam->decide[2]>inParam->decide[3])
			inParam->GMMClass=2;
		else
			inParam->GMMClass=3;
		//__android_log_print(ANDROID_LOG_ERROR, "counter", "%.0f, VAD %d, VADtemp %d,class %d",inParam->Debug[0], VADDec,inParam->VADtemp,inParam->GMMClass);
		//__android_log_print(ANDROID_LOG_ERROR, "counter", "%.0f, decide# %d %d %d %d",inParam->Debug[0],inParam->decide[0],inParam->decide[1],inParam->decide[2],inParam->decide[3]);
	}


	//**LogMMSE**//
	if(inParam->GMMClass>=0){
	LOGMMSE(inParam);//IFFT is inside of LogMMSE, result is stored in the OutputBuffer
	}else{
		for(i=0;i<inParam->stepSize;i++){
			inParam->outputBuffer[i]=inParam->inputBuffer[overlap+i];
		}
	}

	/*if(inParam->Debug[0]>100 && inParam->Debug[0]<110){
		for(i=0;i<10;i++){
		__android_log_print(ANDROID_LOG_ERROR, "counter", "%.0f,in 133+ %f, mfcc %f, ifft %f",inParam->Debug[0],inParam->inputBuffer[133+i],inParam->mfcc[i],inParam->transform->real[i]);
		}
		for(i=0;i<10;i++){
			__android_log_print(ANDROID_LOG_ERROR, "counter", "%.0f, out %f,hw %f,x_old %f",inParam->Debug[0],inParam->outputBuffer[i],inParam->logMMSE->hw[i],inParam->logMMSE->x_old[i]);
		}
	}*/

	stopTimer(inParam->timer);
}

static jlong
initialize(JNIEnv* env, jobject thiz, jint frequency, jint stepsize, jint windowsize, jint decisionBufferLength)
{
	Variables* inParam = (Variables*) malloc(sizeof(Variables));
	inParam->timer = newTimer();
	inParam->frequency = frequency;
	inParam->stepSize = stepsize;
	inParam->windowSize = windowsize;
	inParam->overlap = windowsize-stepsize;
	inParam->inputBuffer = (float*)calloc(windowsize,sizeof(float));
	inParam->outputBuffer = (float*)malloc(stepsize*sizeof(float));
	inParam->originalInput = (short*)malloc(stepsize*sizeof(short));

	inParam->decisionBuffer = (int*)calloc(decisionBufferLength,sizeof(int));
	inParam->decisionBufferLength = decisionBufferLength;
	inParam->decisionBufferCounter = 0;
	inParam->GMMClass=-1;
	inParam->decide[0]=decisionBufferLength;
	inParam->decide[1]=0;
	inParam->decide[2]=0;
	inParam->decide[3]=0;
	inParam->Debug[0]=0;

	//************VAD***************//
	inParam->Ds = 0;
	inParam->Tqb = 0;
	inParam->NoVoiceCount =0;
	inParam->VADflag = 0;
	inParam->VADtemp = 0;
	inParam->Dslength = (int)(frequency/stepsize);////// ??/2
	inParam->xl = (float*) calloc(windowsize/2,sizeof(float));
	inParam->xh = (float*) calloc(windowsize/2,sizeof(float));
	inParam->Dsbuf = (float*) calloc(inParam->Dslength,sizeof(float));
	int point=exp2(ceil(log(inParam->windowSize)/log(2)));



	//************MFCC***************//
	inParam->wts = (float*) calloc((point/2 + 1)*40,sizeof(float));
	inParam->window = (float*) calloc(inParam->windowSize,sizeof(float));
	inParam->FFTbuffer = (float*) calloc(point,sizeof(float));
	inParam->dctm = (float*) calloc(13*40,sizeof(float));
	inParam->mfcc = (float*) calloc(13,sizeof(float));
	inParam->mfcc_pspectrum = (float*) calloc((point/2 + 1),sizeof(float));
	inParam->mfcc_aspectrum_40 = (float*) calloc(40,sizeof(float));



	//MFCC Transform
	inParam->transform = (Transform*)malloc(sizeof(Transform));
	inParam->transform->points = exp2(ceil(log(inParam->windowSize)/log(2)));
	inParam->transform->real = (float*)calloc(inParam->transform->points,sizeof(float));
	inParam->transform->imaginary = (float*)calloc(inParam->transform->points,sizeof(float));
	inParam->transform->sine = (float*)malloc((inParam->transform->points/2)*sizeof(float));
	inParam->transform->cosine = (float*)malloc((inParam->transform->points/2)*sizeof(float));
	float arg;
	int i;
	for (i=0;i<inParam->transform->points/2;i++)
	{
		arg = -2*M_PI*i/inParam->transform->points;
		inParam->transform->cosine[i] = cos(arg);
		inParam->transform->sine[i] = sin(arg);
	}
	hanning(inParam->window,inParam->windowSize);
	fft2melmx(inParam); //result is aspectrum[40]
	spec2cep(inParam);//result is dctm[13*40]



	//************LogMMSE***************//
	inParam->logMMSE = (LogMMSE*)malloc(sizeof(LogMMSE));
	inParam->logMMSE->noise_mean = (float*) calloc(point,sizeof(float));
	inParam->logMMSE->noise_mu2 = (float*) calloc(point,sizeof(float));
	inParam->logMMSE->sig2 = (float*) calloc(point,sizeof(float));
	inParam->logMMSE->Xk_prev = (float*) calloc(point,sizeof(float));
	inParam->logMMSE->Rk_prev = (float*) calloc(point,sizeof(float));
	inParam->logMMSE->NumWinPmin = 0.8 * inParam->frequency / inParam->overlap;
	inParam->logMMSE->SNP = (float**) calloc(inParam->transform->points,sizeof(float*));//inParam->logMMSE->NumWinPmin *
	for(i=0;i<inParam->transform->points;i++){
		inParam->logMMSE->SNP[i]=(float*) calloc(inParam->logMMSE->NumWinPmin,sizeof(float));
	}
	inParam->logMMSE->SPP = (float*) calloc(point,sizeof(float));
	inParam->logMMSE->hw = (float*) calloc(point,sizeof(float));
	inParam->logMMSE->x_old = (float*) calloc(point,sizeof(float));

	__android_log_print(ANDROID_LOG_ERROR, "counter", "finish initialization");


	return (jlong)inParam;
}

static void
finish(JNIEnv* env, jobject thiz, jlong memoryPointer)
{
	Variables* inParam = (Variables*) memoryPointer;
	//cleanup memory
	if(inParam != NULL){
		tellTimerTime(inParam->timer);
		destroyTimer(&(inParam->timer));

		if(inParam->logMMSE != NULL){
			if(inParam->logMMSE->SNP!= NULL){
				int i;
				for(i=0;i<inParam->transform->points;i++){
					if(inParam->logMMSE->SNP[i] != NULL){
						free(inParam->logMMSE->SNP[i]);
						inParam->logMMSE->SNP[i] = NULL;
					}
				}
				free(inParam->logMMSE->SNP);
			}
			if(inParam->logMMSE->noise_mean != NULL){
				free(inParam->logMMSE->noise_mean);}
			if(inParam->logMMSE->noise_mu2 != NULL){
				free(inParam->logMMSE->noise_mu2);}
			if(inParam->logMMSE->sig2 != NULL){
				free(inParam->logMMSE->sig2);}
			if(inParam->logMMSE->Xk_prev != NULL){
				free(inParam->logMMSE->Xk_prev);}
			if(inParam->logMMSE->Rk_prev != NULL){
				free(inParam->logMMSE->Rk_prev);}
			if(inParam->logMMSE->SPP != NULL){
				free(inParam->logMMSE->SPP);}
			if(inParam->logMMSE->hw != NULL){
				free(inParam->logMMSE->hw);}
			if(inParam->logMMSE->x_old != NULL){
				free(inParam->logMMSE->x_old);}
			free(inParam->logMMSE);
			inParam->logMMSE=NULL;
		}
		destroyTransform(&inParam->transform);
		if(inParam->inputBuffer != NULL){
			free(inParam->inputBuffer);
			inParam->inputBuffer = NULL;
		}

		if(inParam->outputBuffer != NULL){
			free(inParam->outputBuffer);
			inParam->outputBuffer = NULL;
		}
		if(inParam->originalInput != NULL){
			free(inParam->originalInput);
			inParam->originalInput = NULL;
		}
		if(inParam->xl != NULL){
			free(inParam->xl);}
		if(inParam->xh != NULL){
			free(inParam->xh);}
		if(inParam->Dsbuf != NULL){
			free(inParam->Dsbuf);}
		if(inParam->wts != NULL){
			free(inParam->wts);}
		if(inParam->window != NULL){
			free(inParam->window);}
		if(inParam->FFTbuffer != NULL){
			free(inParam->FFTbuffer);}
		if(inParam->dctm != NULL){
			free(inParam->dctm);}
		if(inParam->mfcc != NULL){
			free(inParam->mfcc);}
		if(inParam->mfcc_pspectrum != NULL){
			free(inParam->mfcc_pspectrum);}
		if(inParam->mfcc_aspectrum_40 != NULL){
			free(inParam->mfcc_aspectrum_40);}
		free(inParam);
		inParam = NULL;
		__android_log_print(ANDROID_LOG_ERROR, "counter", "finish finish");
	}
}

static jfloat
getTime(JNIEnv* env, jobject thiz, jlong memoryPointer)
{
	Variables* inParam = (Variables*) memoryPointer;
	return getTimerMS(inParam->timer);
}

static jfloatArray
getOutput(JNIEnv* env, jobject thiz, jlong memoryPointer, jint outputSelect)
{
	Variables* inParam = (Variables*) memoryPointer;

	jshortArray output = (*env)->NewShortArray(env, inParam->stepSize);
	short *_output = (*env)->GetShortArrayElements(env, output, NULL);

	if(outputSelect == 0) { //Case 1 - Original input signal
		int i;
		for(i=0;i<inParam->stepSize;i++)
		{
			_output[i] = inParam->originalInput[i];
		}

	} else {				//Case 2 - Processed output signal
		int i;
		for(i=0;i<inParam->stepSize;i++)
		{
			_output[i] = (short)(inParam->outputBuffer[i]*32768.0f);
		}
	}

	(*env)->ReleaseShortArrayElements(env, output, _output, 0);
	return output;
}

static jfloatArray
getDebug(JNIEnv* env, jobject thiz, jlong memoryPointer, jint debugSelect)
{
	Variables* inParam = (Variables*) memoryPointer;

	jfloatArray debugOutput = NULL;

	if(debugSelect == 0) {

		//Test Case 1 - inputBuffer contents

		debugOutput = (*env)->NewFloatArray(env, inParam->windowSize);
		float *_debugOutput = (*env)->GetFloatArrayElements(env, debugOutput, NULL);

		int i;
		for (i=0; i<inParam->windowSize;i++)
		{
			_debugOutput[i] = inParam->inputBuffer[i];
		}

		(*env)->ReleaseFloatArrayElements(env, debugOutput, _debugOutput, 0);

	} else if (debugSelect == 1) {

		//Test Case 2 - outputBuffer contents

		debugOutput = (*env)->NewFloatArray(env, inParam->stepSize);
		float *_debugOutput = (*env)->GetFloatArrayElements(env, debugOutput, NULL);

		int i;
		for (i=0; i<inParam->stepSize;i++)
		{
			_debugOutput[i] = inParam->outputBuffer[i];
		}

		(*env)->ReleaseFloatArrayElements(env, debugOutput, _debugOutput, 0);

	} /*else if (debugSelect == 2) {

		//Test Case 3 - FFT Power Spectrum

		debugOutput = (*env)->NewFloatArray(env, (inParam->fft->points/2)+1);
		float *_debugOutput = (*env)->GetFloatArrayElements(env, debugOutput, NULL);

		int i;
		for (i=0; i<(inParam->fft->points/2)+1;i++)
		{
			_debugOutput[i] = inParam->fft->power[i];
		}

		(*env)->ReleaseFloatArrayElements(env, debugOutput, _debugOutput, 0);

	} else if (debugSelect == 3) {

		//Test Case 4 - FFT Real Portion

		debugOutput = (*env)->NewFloatArray(env, inParam->fft->points);
		float *_debugOutput = (*env)->GetFloatArrayElements(env, debugOutput, NULL);

		int i;
		for (i=0; i<inParam->fft->points;i++)
		{
			_debugOutput[i] = inParam->fft->real[i];//-inParam->fft->window[i];
		}

		(*env)->ReleaseFloatArrayElements(env, debugOutput, _debugOutput, 0);

	} else if (debugSelect == 4) {

		//Test Case 5 - FFT Imaginary Portion

		debugOutput = (*env)->NewFloatArray(env, inParam->fft->points);
		float *_debugOutput = (*env)->GetFloatArrayElements(env, debugOutput, NULL);

		int i;
		for (i=0; i<inParam->fft->points;i++)
		{
			_debugOutput[i] = inParam->fft->imaginary[i];
		}

		(*env)->ReleaseFloatArrayElements(env, debugOutput, _debugOutput, 0);

	} else if (debugSelect == 5) {

		//Test Case 5 - MFCC Filtered FFT Output

		debugOutput = (*env)->NewFloatArray(env, inParam->mfcc->NFilters);
		float *_debugOutput = (*env)->GetFloatArrayElements(env, debugOutput, NULL);

		int i;
		for (i=0; i<(inParam->mfcc->NFilters);i++)
		{
			_debugOutput[i] = inParam->mfcc->MelFFT[i];
		}

		(*env)->ReleaseFloatArrayElements(env, debugOutput, _debugOutput, 0);

	} else if (debugSelect == 6) {

		//Test Case 6 - MFCC Coefficient Output

		debugOutput = (*env)->NewFloatArray(env, CEPSTRAL_COEFFICIENTS);
		float *_debugOutput = (*env)->GetFloatArrayElements(env, debugOutput, NULL);

		int i;
		for (i=0; i<CEPSTRAL_COEFFICIENTS;i++)
		{
			_debugOutput[i] = inParam->mfcc->MelCoeffOut[i];
		}

		(*env)->ReleaseFloatArrayElements(env, debugOutput, _debugOutput, 0);

	} else if (debugSelect == 7) {

		//Test Case 7 - VAD High Filtered

		debugOutput = (*env)->NewFloatArray(env, inParam->stepSize);
		float *_debugOutput = (*env)->GetFloatArrayElements(env, debugOutput, NULL);

		int i;
		for (i=0; i<(inParam->stepSize);i++)
		{
			_debugOutput[i] = inParam->vad->high->result[i];
		}

		(*env)->ReleaseFloatArrayElements(env, debugOutput, _debugOutput, 0);

	} else if (debugSelect == 8) {

		//Test Case 8 - VAD Low Filtered

		debugOutput = (*env)->NewFloatArray(env, inParam->stepSize);
		float *_debugOutput = (*env)->GetFloatArrayElements(env, debugOutput, NULL);

		int i;
		for (i=0; i<(inParam->stepSize);i++)
		{
			_debugOutput[i] = inParam->vad->low->result[i];
		}

		(*env)->ReleaseFloatArrayElements(env, debugOutput, _debugOutput, 0);

	} else if (debugSelect == 9) {

		//Test Case 9 - Ds Buffer

		debugOutput = (*env)->NewFloatArray(env, inParam->frequency/inParam->stepSize);
		float *_debugOutput = (*env)->GetFloatArrayElements(env, debugOutput, NULL);

		int i;
		for (i=0; i<(inParam->frequency/inParam->stepSize);i++)
		{
			_debugOutput[i] = inParam->vad->DsBuffer[i];
		}

		(*env)->ReleaseFloatArrayElements(env, debugOutput, _debugOutput, 0);

	} else if (debugSelect == 10) {

		//Test Case 9 - Ds Buffer Sorted

		debugOutput = (*env)->NewFloatArray(env, inParam->frequency/inParam->stepSize);
		float *_debugOutput = (*env)->GetFloatArrayElements(env, debugOutput, NULL);

		int i;
		for (i=0; i<(inParam->frequency/inParam->stepSize);i++)
		{
			_debugOutput[i] = inParam->vad->DsBufferSorted[i];
		}

		(*env)->ReleaseFloatArrayElements(env, debugOutput, _debugOutput, 0);

	} else if (debugSelect == 11) {

		//Test Case 11 - VAD Outputs (Decision, Dc, Tqb)

		debugOutput = (*env)->NewFloatArray(env, 3);
		float *_debugOutput = (*env)->GetFloatArrayElements(env, debugOutput, NULL);

		_debugOutput[0] = inParam->vad->vadDecision;
		_debugOutput[1] = inParam->vad->previousDs;
		_debugOutput[2] = inParam->vad->previousTqb;

		(*env)->ReleaseFloatArrayElements(env, debugOutput, _debugOutput, 0);

	} else if (debugSelect == 12) {

		//Test Case 12 - Classifier Outputs (Decision, (Class count, Probability))

		debugOutput = (*env)->NewFloatArray(env, 1+2*inParam->classifier->classes);
		float *_debugOutput = (*env)->GetFloatArrayElements(env, debugOutput, NULL);

		_debugOutput[0] = inParam->classifier->classDecision;

		int i;
		for(i=1;i<=inParam->classifier->classes;i++)
		{
			_debugOutput[i] = inParam->classifier->classDecisionCount[i-1];
		}

		for(i=1;i<=inParam->classifier->classes;i++)
		{
			_debugOutput[i+inParam->classifier->classes] = inParam->classifier->gmmClassifiers[i-1]->probability;
		}

		(*env)->ReleaseFloatArrayElements(env, debugOutput, _debugOutput, 0);
	}*/

	//Add additional cases to output other data that may be needed.

	return debugOutput;
}

////////////////////////////////////////////////////////////////////////////////////////////
// JNI Setup - Functions and OnLoad
////////////////////////////////////////////////////////////////////////////////////////////

static JNINativeMethod nativeMethods[] =
	{//		Name							Signature											Pointer
			{"compute", 					"(J[S)V",											(void *)&compute				},
			{"initialize",					"(IIII)J",											(void *)&initialize				},
			{"finish",						"(J)V",												(void *)&finish					},
			{"getTime",						"(J)F",												(void *)&getTime				},
			{"getOutput",					"(JI)[S",											(void *)&getOutput				},
			{"getDebug",					"(JI)[F",											(void *)&getDebug				}
	};

jint
JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env;
	jint result;
	//get a hook to the environment
	result = (*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_6);
	if (result == JNI_OK) {
		//find the java class to hook the native methods to
		jclass filters = (*env)->FindClass(env, "com/dsp/speechpipeline/SpeechProcessing");
		if (filters != NULL) {
			result = (*env)->RegisterNatives(env, filters, nativeMethods, sizeof(nativeMethods)/sizeof(nativeMethods[0]));
			(*env)->DeleteLocalRef(env, filters);
			if(result == JNI_OK){
				return JNI_VERSION_1_6;
			} else {
				//something went wrong with the method registration
				return JNI_ERR;
			}
		} else {
			//class wasn't found
			return JNI_ERR;
		}
	} else {
		//could not get environment
		return JNI_ERR;
	}
}
