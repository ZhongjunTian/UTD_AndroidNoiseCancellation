package com.dsp.speechpipeline;

import java.util.Arrays;

public class WaveFrame {
	private float[] debugData;
	private short[] audioSamples;
	
	public WaveFrame(short[] ssamples){
		this.debugData = null;
		this.audioSamples = Arrays.copyOf(ssamples, ssamples.length);
	}
	
	public void setDebug(float[] debugdata){
		this.debugData = debugdata;
	}
	
	public float[] getDebug(){
		return debugData;
	}
	
	public short[] getAudio(){ 
		return audioSamples;
	}
	
	public void setAudioSamples(short[] samples){
		this.audioSamples = samples;
	}
}
