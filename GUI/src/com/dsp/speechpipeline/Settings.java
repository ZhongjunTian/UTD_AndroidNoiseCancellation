package com.dsp.speechpipeline;

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
	public static boolean output = false;
	public static boolean changed = false;
	public static int debugLevel = 4;
	public static int debugOutput = 0;
	private static Monitor main;
	
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
	
	public static boolean setStepSize(float steptime){
		if (steptime > 0) {
			int stepsize = Math.round(steptime*Fs*0.001f);
			if (stepSize != stepsize && stepsize <= windowSize) {
				stepSize = stepsize;
				stepTime = steptime;
				secondConstant = Fs/stepsize;
				changed = true;
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
			return true;
		}
		return false;
	}
	
	public static boolean setPlayback(boolean flag){
		if(playback != flag){
			playback = flag;
			changed = true;
			return true;
		}
		return false;
	}
	
	public static boolean setOutput(int stream){
		if((stream == 1) && (output != true)) { 		//original signal
			output = true;
			changed = true;
			return true;
		} else if((stream == 2) && (output != false)) { //filtered signal
			output = false;
			changed = true;
			return true;
		}
		return false;
	}
	
	public static String getOutput(){
		if (output == true){
			return "Original";
		} else {
			return "Filtered";
		}
	}
	
	public static String getDebugLevel(){
		if(debugLevel == 4){
			return "None";
		} else if (debugLevel == 3){
			return "PCM";
		} else if(debugLevel == 2){
			return "Text";
		} else if(debugLevel == 1){
			return("All");
		}
		return "How could you let this happen?";
	}
	
	public static boolean setDebugLevel(int level){
		if(debugLevel < 1 || debugLevel > 4 ){
			return false;
		} else if (debugLevel != level){
			debugLevel = level;
			changed = true;
			return true;
		}
		return false;
	}
	
	//Text debugging outputs. These are defined in arrays.xml
	//Names correspond to values in debugTextOptions. Integer
	//assignments correspond to values in debugTextValues.
	//The checking logic in setDebugOutput should match with 
	//the total number of options you wish to implement.
	public static String getDebugOutput(){
		if (debugLevel > 2) {
			return "None";
		} else if(debugOutput == 0){
			return "inputBuffer";
		} else if (debugOutput == 1){
			return "outputBuffer";
		}/* else if(debugOutput == 2){
			return "FFT Power";
		} else if(debugOutput == 3){
			return("FFT Real");
		}  else if(debugOutput == 4){
			return("FFT Imaginary");
		} else if(debugOutput == 5){
			return("Mel Coeffs");
		} ... etc */
		return "You've done it now!";
	}
	
	public static boolean setDebugOutput(int output){
		if(debugOutput < 0 || debugOutput > 2) {
			return false;
		} else if(debugOutput != output){
			debugOutput = output;
			changed = true;
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
