#include <jni.h>
#include "SpeechProcessing.h"


static void
compute(JNIEnv *env, jobject thiz,  jlong memoryPointer, jshortArray input)
{
	Variables* inParam = (Variables*) memoryPointer;
	start(inParam->timer);

	short *_in = (*env)->GetShortArrayElements(env, input, NULL);

	int i, overlap, stepsize;
	overlap = inParam->overlap;
	stepsize = inParam->stepSize;
	float* inputBuffer=inParam->inputBuffer;// there places is different! when we extend the buffer
	/*for(i=0; i<11; i++)
	{
		inParam->inputBuffer[i]=inputBuffer[i];
	}*/
	for(i=0; i<overlap; i++)
	{
		inputBuffer[i] = inputBuffer[stepsize + i];
	}

	for (i=0; i<stepsize; i++)
	{
		inputBuffer[overlap + i] =(float)(_in[i]/32768.0f);
	}

	(*env)->ReleaseShortArrayElements(env, input, _in, 0);

	//Processing logic here. For now this placeholder will copy the input
	//to the output array for the processing function and scale it by 1/2.
	//For switching between suppressed and original output a GUI option
	//can be used to select the output channel.

	int VADDec=VAD(inputBuffer+overlap,inParam);
	/*
	 for(i=0;i<inParam->windowSize;i++){
		inParam->transform->real[i]=inputBuffer[i];
	}
	for(i=inParam->windowSize;i<inParam->transform->points;i++){
		inParam->transform->real[i]=0;
	}
	FFT(inParam->transform);
*/
	for(i=0;i<stepsize;i++)
	{
		inParam->outputBuffer[i] = VADDec;//inParam->inputBuffer[overlap+i]*0.5f;
	}

	stop(inParam->timer);
}

static jlong
initialize(JNIEnv* env, jobject thiz, jint freq, jint stepsize, jint windowsize)
{
	Variables* inParam = (Variables*) malloc(sizeof(Variables));
	inParam->timer = newTimer();
	inParam->frequency = freq;
	inParam->stepSize = stepsize;
	inParam->windowSize = windowsize;
	inParam->overlap = windowsize-stepsize;
	inParam->Ds = 0;
	inParam->Tqb = 0;
	inParam->NoVoiceCount =0;
	inParam->VADflag = 0;
	inParam->Dslength = (int)(freq/windowsize/2);
	inParam->inputBuffer = (float*) calloc(inParam->windowSize,sizeof(float));
	inParam->outputBuffer = (float*) malloc(stepsize*sizeof(float));
	inParam->xl = (float*) calloc(windowsize/2,sizeof(float));
	inParam->xh = (float*) calloc(windowsize/2,sizeof(float));
	inParam->Dsbuf = (float*) calloc((int)(freq/(windowsize/2)),sizeof(float));
	int i;
	for(i=0;i<10;i++){
	inParam->Debug[i]=0;
	}

	inParam->transform = (Transform*)malloc(sizeof(Transform));
	inParam->transform->points = exp2(ceil(log(inParam->windowSize)/log(2)));
	inParam->transform->real = (float*)calloc(inParam->transform->points,sizeof(float));
	inParam->transform->imaginary = (float*)calloc(inParam->transform->points,sizeof(float));
	inParam->transform->sine = (float*)malloc((inParam->transform->points/2)*sizeof(float));
	inParam->transform->cosine = (float*)malloc((inParam->transform->points/2)*sizeof(float));
	float arg;
	for (i=0;i<inParam->transform->points/2;i++)
	{
		arg = -2*M_PI*i/inParam->transform->points;
		inParam->transform->cosine[i] = cos(arg);
		inParam->transform->sine[i] = sin(arg);
	}

	return (jlong)inParam;
}

static void
finish(JNIEnv* env, jobject thiz, jlong memoryPointer)
{
	Variables* inParam = (Variables*) memoryPointer;
	//cleanup memory
	if(inParam != NULL){
		tellTime(inParam->timer);
		destroy(&(inParam->timer));
		if(inParam->inputBuffer != NULL){
			free(inParam->inputBuffer);
			inParam->inputBuffer = NULL;
		}
		if(inParam->outputBuffer != NULL){
			free(inParam->outputBuffer);
			inParam->outputBuffer = NULL;
		}
		if(inParam->xl != NULL){
			free(inParam->xl);
			inParam->xl = NULL;
		}
		if(inParam->xh != NULL){
			free(inParam->xh);
			inParam->xh = NULL;
		}
		if(inParam->Dsbuf != NULL){
			free(inParam->Dsbuf);
			inParam->Dsbuf = NULL;
		}
		destroyTransform(&(inParam->transform));
		free(inParam);
		inParam = NULL;
	}
}

static jfloat
getTime(JNIEnv* env, jobject thiz, jlong memoryPointer)
{
	Variables* inParam = (Variables*) memoryPointer;
	return getMS(inParam->timer);
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
			_output[i] = (short)(inParam->inputBuffer[inParam->overlap+i]*32768);
		}

	} else {				//Case 2 - Processed output signal
							//This should be your synthesized output.
		int i;
		for(i=0;i<inParam->stepSize;i++)
		{
			_output[i] = inParam->outputBuffer[i];
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

	if(debugSelect == 0) { 			//Test Case 1 - inputBuffer contents

		debugOutput = (*env)->NewFloatArray(env, inParam->windowSize);
		float *_debugOutput = (*env)->GetFloatArrayElements(env, debugOutput, NULL);

		int i;
		for (i=0; i<inParam->windowSize;i++)
		{
			_debugOutput[i] = inParam->inputBuffer[i];
		}

		(*env)->ReleaseFloatArrayElements(env, debugOutput, _debugOutput, 0);
	} else if (debugSelect == 1) {	//Test Case 2 - outputBuffer contents

		debugOutput = (*env)->NewFloatArray(env, inParam->stepSize);
		float *_debugOutput = (*env)->GetFloatArrayElements(env, debugOutput, NULL);

		int i;
		for (i=0; i<inParam->stepSize;i++)
		{
			_debugOutput[i] = inParam->outputBuffer[i];
		}

		(*env)->ReleaseFloatArrayElements(env, debugOutput, _debugOutput, 0);
	}

	//Add additional cases to output other data that may be needed.

	return debugOutput;
}

////////////////////////////////////////////////////////////////////////////////////////////
// JNI Setup - Functions and OnLoad
////////////////////////////////////////////////////////////////////////////////////////////

static JNINativeMethod nativeMethods[] =
	{//		Name							Signature			Pointer
			{"compute", 					"(J[S)V",			(void *)&compute				},
			{"initialize",					"(III)J",			(void *)&initialize				},
			{"finish",						"(J)V",				(void *)&finish					},
			{"getTime",						"(J)F",				(void *)&getTime				},
			{"getOutput",					"(JI)[S",			(void *)&getOutput				},
			{"getDebug",					"(JI)[F",			(void *)&getDebug				}
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
