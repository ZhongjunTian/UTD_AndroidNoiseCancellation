package com.dsp.speechpipeline;

import java.util.Locale;
import java.util.concurrent.atomic.AtomicInteger;

import android.content.res.AssetManager;
import android.media.AudioFormat;
import android.media.MediaRecorder;

public class Settings {
	
	public static final int FORMAT = AudioFormat.ENCODING_PCM_16BIT;
	public static final int SOURCE = MediaRecorder.AudioSource.DEFAULT;
	public static final WaveFrame STOP = new WaveFrame(new short[] {1,2,4,8});
	public static int Fs = 8000;
	public static float stepTime = 5.0f; 								//default step time
	public static float windowTime = 11.0f; 							//default window time
	public static int stepSize = Math.round(stepTime*Fs*0.001f);		//step in terms of samples
	public static int windowSize = Math.round(windowTime*Fs*0.001f);	//window in terms of samples
	public static boolean playback = false;
	public static AtomicInteger output = new AtomicInteger();
	public static boolean changed = false;
	public static int debugLevel = 0;
	public static int debugOutput = 0;
	public static int decisionBufferLength = 200;
	public static String[] debugOutputNames = {"Default"};
	public static String[] debugLevels = {"Default"};
	public static String[] audioOutputs = {"Default"};
	public static String[] noiseClasses = {"Unavailable", "Machinery", "Babble", "Car"};
	private static Monitor main;
	private static AssetManager assetManager;
	
	// UI update interval
	public static int secondConstant = Fs/stepSize;
	
	//supported sampling rates
	public static CharSequence[] samplingRates = {"8000 Hz"};
	public static CharSequence[] samplingRateValues = {"8000"};
	
	public static void setCallbackInterface(Monitor uiInterface) {
		main = uiInterface;
	}
	
	public static Monitor getCallbackInterface() {
		return main;
	}
	
	public static AssetManager getAssetManager() {
		return assetManager;
	}

	public static void setAssetManager(AssetManager assetManager) {
		Settings.assetManager = assetManager;
	}

	public static boolean setStepSize(float steptime){
		if (steptime > 0) {
			int stepsize = Math.round(steptime*Fs*0.001f);
			if (stepSize != stepsize && stepsize <= windowSize) {
				stepSize = stepsize;
				stepTime = steptime;
				secondConstant = Fs/stepsize;
				changed = true;
				main.notify("Step time set to " + Settings.stepTime + "ms.");
				return true;
			}
		}
		return false;
	}
	
	public static boolean setWindowSize(float windowtime){
		if(windowtime > 0) {
			int windowsize = Math.round(windowtime*Fs*0.001f);
			if (windowSize != windowsize && windowsize >= stepSize) {
				windowSize = windowsize;
				windowTime = windowtime;
				changed = true;
				main.notify("Window time set to " + Settings.windowTime + "ms.");
				return true;
			}
		}
		return false;
	}
	
	public static boolean setSamplingFrequency(int freq){
		if(Fs != freq){
			Fs = freq;
			stepSize = Math.round(stepTime*Fs*0.001f);
			windowSize = Math.round(windowTime*Fs*0.001f);
			secondConstant = Fs/stepSize;
			changed = true;
			main.notify("Sampling rate set to " + Settings.Fs + "Hz.");
			return true;
		}
		return false;
	}
	
	public static boolean setDecisionBufferLength(int length) {
		if(length > 0 && length != decisionBufferLength) {
			decisionBufferLength = length;
			changed = true;
			main.notify("Decision buffer length set to " + decisionBufferLength + " frames.");
			return true;
		}
		return false;
	}

	public static boolean setPlayback(boolean flag){
		if(playback != flag){
			playback = flag;
			changed = true;
			String[] result = {"Playback disabled.", "Playback enabled for " + getOutput().toLowerCase(Locale.US) + " output."};
			main.notify(result[Settings.playback?1:0]);
			return true;
		}
		return false;
	}
	
	public static boolean setOutput(int stream){
		if(stream < 0 || stream > audioOutputs.length) {
			return false;
		} else if(stream != output.getAndSet(stream)) {
			changed = true;
			String[] result = {"Playback set to original output.", "Playback set to filtered output."};
			main.notify(result[output.get()]);
			return true;
		}
		return false;
	}
	
	public static String getOutput(){
		return audioOutputs[output.get()];
	}
	
	public static String getDebugLevel(){
		return debugLevels[debugLevel];
	}
	
	public static boolean setDebugLevel(int level){
		if(level < 0 || level > debugLevels.length){
			return false;
		} else if (debugLevel != level){
			debugLevel = level;
			changed = true;
			String[] result = {"Debug ouput disabled.", "Classification output enabled.", "Text file output enabled.", "PCM ouput enabled.", "All debug outputs enabled."};
			main.notify(result[debugLevel]);
			return true;
		}
		return false;
	}
	
	public static String getDebugOutput(){
		return debugOutputNames[debugOutput];
	}
	
	public static boolean setDebugOutput(int output){
		if(output < 0 || output > debugOutputNames.length) {
			return false;
		} else if(debugOutput != output){
			debugOutput = output;
			changed = true;
			main.notify(getDebugOutput() + " text output selected.");
			return true;
		}
		return false;
	}
	
	public static void setRates(CharSequence[] rates){
		samplingRates = rates;
	}
	
	public static void setRateValues(CharSequence[] rateValues){
		samplingRateValues = rateValues;
	}
}
