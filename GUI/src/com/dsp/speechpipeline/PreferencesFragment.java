package com.dsp.speechpipeline;

import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceFragment;

public class PreferencesFragment extends PreferenceFragment implements OnSharedPreferenceChangeListener {

	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		//preference options are defined in res/xml/prefs.xml
		//preference names are defined in res/values/strings.xml
 		addPreferencesFromResource(R.xml.prefs);
 		
 		//sound output option
 		Preference preference = findPreference(getString(R.string.prefPlayback));
 		preference.setSummary("Current: " + (Settings.playback?"Enabled":"Disabled"));
 		
 		//sound channel option
 		preference = findPreference(getString(R.string.prefOutputStream));
 		preference.setSummary("Current: " + Settings.getOutput());
 		
 		//sampling frequency options
        preference = findPreference(getString(R.string.prefSamplingFreq));
        ((ListPreference)preference).setEntries(Settings.samplingRates);
        ((ListPreference)preference).setEntryValues(Settings.samplingRateValues);
        preference.setSummary("Current: " + Settings.Fs + "Hz");
        
        //window time option
        preference = findPreference(getString(R.string.prefWindowTime));
        preference.setSummary("Current: " + Settings.windowTime + "ms");
        preference.setOnPreferenceChangeListener(preferenceChange);
        
        //step time option
        preference = findPreference(getString(R.string.prefStepTime));
        preference.setSummary("Current: " + Settings.stepTime + "ms");
        preference.setOnPreferenceChangeListener(preferenceChange);
        
        //decision buffer length option
        preference = findPreference(getString(R.string.prefDecisionBufferLength));
        preference.setSummary("Current: " + Settings.decisionBufferLength + " frames");
        preference.setOnPreferenceChangeListener(preferenceChange);
        
        //debug output option
        preference = findPreference(getString(R.string.prefDebug));
        preference.setSummary("Current: " + Settings.getDebugLevel());
        
        //debug text output option
        preference = findPreference(getString(R.string.prefDebugOutput));
        preference.setSummary("Current: " + Settings.getDebugOutput());
	}

	public void onResume() {
		super.onResume();
		getPreferenceManager().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
	}
	
	public void onPause() {
		getPreferenceManager().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(this);
		super.onPause();
	}
	
	private OnPreferenceChangeListener preferenceChange = new OnPreferenceChangeListener() {
		public boolean onPreferenceChange(Preference preference, Object changedValue) {
			String testString = changedValue.toString().trim();
			if(testString == "") {
				return false;
			} else {
				try {
					Float.parseFloat(testString);
					return true;
				} catch(NumberFormatException e){}
				return false;
			}
		}
	};

	public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
		
		Preference preference = findPreference(key);
		
		//sound output option
		if (key.equals(getString(R.string.prefPlayback))) {
			boolean setting = sharedPreferences.getBoolean(key, false);
			if(Settings.setPlayback(setting)) {
				preference.setSummary("Current: " + (Settings.playback?"Enabled":"Disabled"));
				preference = findPreference(getString(R.string.prefOutputStream));
		 		preference.setSummary("Current: " + Settings.getOutput()); 	
			}
		}
		
		//sound channel option
		else if (key.equals(getString(R.string.prefOutputStream))) {
			int setting = ((ListPreference)preference).findIndexOfValue(sharedPreferences.getString(key, Settings.audioOutputs[0]));
			if(Settings.setOutput(setting)) {
				preference.setSummary("Current: " + Settings.getOutput());
			}
		}
		
		//sampling frequency options
		else if (key.equals(getString(R.string.prefSamplingFreq))) {
			int setting = Integer.parseInt(sharedPreferences.getString(key, "8000"));
			if(Settings.setSamplingFrequency(setting)) {
				preference.setSummary("Current: " + Settings.Fs + "Hz");
			}
		}
		
		//window time option
		else if (key.equals(getString(R.string.prefWindowTime))) {
			float setting = Float.parseFloat(sharedPreferences.getString(key, "11.0"));
			if(Settings.setWindowSize(setting)) {
				preference.setSummary("Current: " + Settings.windowTime + "ms");
			}
		}
		
		//step time option
		else if (key.equals(getString(R.string.prefStepTime))) {
			float setting = Float.parseFloat(sharedPreferences.getString(key, "5.0"));
			if(Settings.setStepSize(setting)) {
				preference.setSummary("Current: " + Settings.stepTime + "ms");
			}
		}
		
		//decision buffer length option
		else if (key.equals(getString(R.string.prefDecisionBufferLength))) {
			float setting = Float.parseFloat(sharedPreferences.getString(key, "200"));
			if(Settings.setDecisionBufferLength((int)setting)) {
				preference.setSummary("Current: " + Settings.decisionBufferLength + " frames");
			}
		}
		
		//debug output option
		else if (key.equals(getString(R.string.prefDebug))) {
			int setting = ((ListPreference)preference).findIndexOfValue((sharedPreferences.getString(key, Settings.debugLevels[0])));
			if(Settings.setDebugLevel(setting)) {
				preference.setSummary("Current: " + Settings.getDebugLevel());
				preference = findPreference(getString(R.string.prefDebugOutput));
		 		preference.setSummary("Current: " + Settings.getDebugOutput()); 	
			}
		}
		
		//debug text output option
		else if (key.equals(getString(R.string.prefDebugOutput))) {
			int setting = ((ListPreference)preference).findIndexOfValue((sharedPreferences.getString(key, Settings.debugOutputNames[0])));
			if(Settings.setDebugOutput(setting)) {
				preference.setSummary("Current: " + Settings.getDebugOutput());
			}
		}
	}
}