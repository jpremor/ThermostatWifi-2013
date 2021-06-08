package com.wifi.thermostat;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.View;
import android.widget.CheckBox;
import android.widget.EditText;

public class SettingsActivity extends Activity {

	protected static final int RESULT_OK_DIALOG_SETT = 14;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		//setupActionBar();
		setContentView(R.layout.setup_layout);
		
		CheckBox CheckboxCelc = (CheckBox) findViewById(R.id.checkBoxDegCelc);
		EditText EditServer = (EditText) findViewById(R.id.editTextServer);
		EditText EditPort = (EditText) findViewById(R.id.editTextPort);
		CheckBox CheckboxLocalNet = (CheckBox) findViewById(R.id.checkBoxLocalNet);
		
		SharedPreferences prefs = getSharedPreferences("connection-settings",MODE_PRIVATE);
		
		CheckboxCelc.setChecked(prefs.getBoolean("deg-celcius", false));
		EditServer.setText(prefs.getString("server-ip", "none"));
		EditPort.setText(String.valueOf(prefs.getInt("server-port", 0)));
		CheckboxLocalNet.setChecked(prefs.getBoolean("server-localnet", true));
	}	
	
	
	public void BtnCancelOnClick(View View){		
        finish();
   	}

	public void BtnOkOnClick(View View){		
		CheckBox CheckboxCelc = (CheckBox) findViewById(R.id.checkBoxDegCelc);
		EditText EditServer = (EditText) findViewById(R.id.editTextServer);
		EditText EditPort = (EditText) findViewById(R.id.editTextPort);
		CheckBox CheckboxLocalNet = (CheckBox) findViewById(R.id.checkBoxLocalNet);
		
    	Intent resultIntent = new Intent();
    	resultIntent.putExtra("setting_deg_c", CheckboxCelc.isChecked());
    	resultIntent.putExtra("setting_new_server", EditServer.getText().toString());
    	resultIntent.putExtra("setting_new_port", Integer.parseInt(EditPort.getText().toString()));
    	resultIntent.putExtra("setting_local_net", CheckboxLocalNet.isChecked());
    	
    	
    	setResult(RESULT_OK_DIALOG_SETT, resultIntent);
    	
        finish();
   	}

}
