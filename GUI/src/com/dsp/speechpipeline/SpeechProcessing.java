package com.dsp.speechpipeline;

public class SpeechProcessing {
	
	long pointer;
	
	public SpeechProcessing(){
		pointer = initialize(Settings.Fs, Settings.stepSize, Settings.windowSize);
	}
	
	public void release(){
		finish(pointer);
	}
	
	public void process(short[] in){
		compute(pointer, in);
	}
	
	public float getTime(){
		return getTime(pointer);
	}
	
	public float[] getDebug(){
		return getDebug(pointer, Settings.debugOutput);
	}
	
	public short[] getOutput(){
		return getOutput(pointer, Settings.output?0:1);
	}

	//JNI Method Calls	
	private static native void compute(long memoryPointer, short[] in);
	private static native long initialize(int frequency, int stepSize, int windowSize);
	private static native void finish(long memoryPointer);
	private static native float getTime(long memoryPointer);
	private static native float[] getDebug(long memoryPointer, int debugOutput);
	private static native short[] getOutput(long memoryPointer, int outputSelect);

}
