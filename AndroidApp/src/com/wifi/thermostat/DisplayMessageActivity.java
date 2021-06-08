package com.wifi.thermostat;

import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.view.View;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

public class DisplayMessageActivity extends Activity {

	//private boolean no_changes = true;
	
	private static final int DEFAULT_MIN_PROGRESS = 100;

	
	

	//@SuppressLint("NewApi")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_display_message);
		
		TextView TextTitle = (TextView) findViewById(R.id.textViewDialogTitle);
		TextView TextAdjTemp = (TextView) findViewById(R.id.textViewAdjustment);
		SeekBar AdjBar = (SeekBar) findViewById(R.id.seekBar);
		
		AdjBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {       
		    @Override       
		    public void onStopTrackingTouch(SeekBar seekBar) { 
		    }       

		    @Override       
		    public void onStartTrackingTouch(SeekBar seekBar) {
		    }       

		    @Override       
		    public void onProgressChanged(SeekBar seekBar, int progress,boolean fromUser) {
		    	Bundle extras = getIntent().getExtras();
		      	int numberMin = extras.getInt("dialog_min_str",DEFAULT_MIN_PROGRESS);
		     	
		    	SeekBar AdjBar = (SeekBar) findViewById(R.id.seekBar);
		    	TextView TextAdjTemp = (TextView) findViewById(R.id.textViewAdjustment);
				Float ftemp = ((float)(AdjBar.getProgress() + (float)numberMin ) / 10);
				
				TextAdjTemp.setText( String.valueOf(ftemp) );
				//no_changes = false;
		    }
		});

		// Get the message from the intent
	    //Intent intent = getIntent();
	    Bundle extras = getIntent().getExtras();
	    if (extras != null) {
	    	String dialog_edit = extras.getString("dialog_title_str");
	    	String number_edit = extras.getString("dialog_editn_str");
	    	int numberMin = extras.getInt("dialog_min_str",DEFAULT_MIN_PROGRESS);
	    	int numberMax = extras.getInt("dialog_max_str",500);
	    	float numberEdit = 0;
	    	try {
				numberEdit = Float.parseFloat(number_edit);
	    	} catch (NumberFormatException nfe) {
	    		numberEdit = 25;
	    	}
				    	
	    	if (dialog_edit != null) TextTitle.setText(dialog_edit);
	    	if (number_edit != null) {
	    		AdjBar.setMax(numberMax - numberMin);
		    	AdjBar.setProgress(Math.round((numberEdit) * 10) - numberMin);
	    		TextAdjTemp.setText(number_edit);	    		
	    	}
	    }	    
	    
	    // Set the text view as the activity layout
	    //setContentView(TextTitle);
		
	}

	protected static final int RESULT_CANCEL_DIALOG = 12;
	protected static final int RESULT_OK_DIALOG = 13;
	
	
	public void BtnCancelOnClick(View View){	
		Intent resultIntent = new Intent();
    	setResult(RESULT_CANCEL_DIALOG, resultIntent);
    	
        finish();
   	}

	public void BtnOkOnClick(View View){		
		SeekBar AdjBar = (SeekBar) findViewById(R.id.seekBar);
		Bundle extras = getIntent().getExtras();
      	int numberMin = extras.getInt("dialog_min_str",DEFAULT_MIN_PROGRESS);
     	
    	Intent resultIntent = new Intent();
    	resultIntent.putExtra("dialog_edit_result", AdjBar.getProgress() + numberMin);
    	
    	setResult(RESULT_OK_DIALOG, resultIntent);
    	
        finish();
   	}
	
}
