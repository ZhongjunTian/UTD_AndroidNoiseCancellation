package com.dsp.speechpipeline;

import java.util.concurrent.BlockingQueue;

public class Processing implements Runnable{
	
	private BlockingQueue<WaveFrame> input;
	private BlockingQueue<WaveFrame> output;
	private Thread speechThread;
	private SpeechProcessing speechProcessor;
	private int counter;
	
	public Processing(BlockingQueue<WaveFrame> input, BlockingQueue<WaveFrame> output) {
		this.input = input;
		this.output = output;
		this.counter = 0;
		speechProcessor = new SpeechProcessing();
        speechThread = new Thread(this);
        speechThread.start();
	}

	public void run() {
		try {
			loop:while(true) {
				WaveFrame currentFrame = null;
				currentFrame = input.take();
				if(currentFrame == Settings.STOP){
					Settings.getCallbackInterface().notify("Overall Average Frame Time: " + speechProcessor.getTime() + " ms");
					speechProcessor.release();
					output.put(currentFrame);
					break loop;
				}
				speechProcessor.process(currentFrame.getAudio());
				if(Settings.playback || Settings.debugLevel == 3 || Settings.debugLevel == 1) {
					currentFrame.setAudioSamples(speechProcessor.getOutput());
				}
				if(Settings.debugLevel<3){
					currentFrame.setDebug(speechProcessor.getDebug());
				}
				if(counter == Settings.secondConstant){
					Settings.getCallbackInterface().notify("Average Frame Time: " + speechProcessor.getTime() + " ms");
					counter = 0;
				} else {
					counter++;
				}
				output.put(currentFrame);
			}
		} catch (InterruptedException e) {
			Thread.currentThread().interrupt();
			e.printStackTrace();
		}
	}
}