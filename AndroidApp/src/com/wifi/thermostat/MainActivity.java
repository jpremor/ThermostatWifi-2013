package com.wifi.thermostat;

import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.AsyncTask;
import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Typeface;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ListView;
import android.widget.Switch;
import android.widget.TextView;


public class MainActivity extends Activity {

    private Timer TimerUpdateUI;
	private Timer TimerReadData;
		
	public final static String EDIT_DIALOG_MSG = "com.wifi.thermostat.EDIT_MSG";
	protected static final int RESULT_CANCEL_DIALOG = 12;
	protected static final int RESULT_OK_DIALOG = 13;
	protected static final int RESULT_OK_DIALOG_SETT = 14;
		
	
	protected static final int SYSTEMST_REQFULLSTATE = 2;
	protected static final int SYSTEMST_CONNECTING = 3;
	protected static final int SYSTEMST_WAITINGANS = 4;
	protected static final int SYSTEMST_WAITINGREQS = 5;
	private int SystemState = SYSTEMST_WAITINGREQS;
	
	private TCPClient mTcpClient;
	public String SERVERIP = "remorhome.no-ip.org";//"192.168.1.100"; //your computer IP address
    public int SERVERPORT = 2001;
    public boolean DEGCELCIUS = false;
    public boolean LOCALNETWORK = true;
    public static final String DEFAULTREFCOOL = "24.0";
    public static final String DEFAULTREFHEAT = "18.0";
	private ListView mList;
    private ArrayList<String> arrayList;
    private CustomAdapter mAdapter;

    private boolean ReqsCoolRefUpdate = false;
    private boolean ReqsHeatRefUpdate = false;
    private boolean ReqsFullStatusRequest = false;
    private boolean ReqsSetCtrlEnable = false;
    private boolean ReqsSetForceFan = false;
    
    private boolean waitingCoolDialogResult = false;
    private boolean waitingHeatDialogResult = false;
    
    
    float FartoCelc(float Far) {
    	return ( (Far - 32) * 5 / 9 ); 
    }
    
    float CelctoFar(float Celc) {
    	return ( (Celc * 9 / 5) + 32 ); 
    }
    

    void ReadPreferences() {

		// //////////////////
		// shared preferences
	    SharedPreferences prefs = getSharedPreferences("connection-settings",MODE_PRIVATE);
	    
	    SERVERIP = prefs.getString("server-ip", null);
	    SERVERPORT = prefs.getInt("server-port", -1);	
		DEGCELCIUS = prefs.getBoolean("deg-celcius", false);
	    LOCALNETWORK = prefs.getBoolean("server-localnet", true);

	    SharedPreferences.Editor editor = getSharedPreferences("connection-settings",MODE_PRIVATE).edit();
	    boolean need_commit = false;
	    if (SERVERIP == null) { 
	    	editor.putString("server-ip", "192.168.1.100");
	    	need_commit = true;
	    	SERVERIP = "192.168.1.100";
	    }
	    if (SERVERPORT == -1) { 
	    	editor.putInt("server-port", 2001);
	    	need_commit = true;
	    	SERVERPORT = 2001;
	    }
	    
	    if (need_commit) editor.commit();

    }
    
    void InitTimers() {
    	// //////////////////////
    	// initialize timer
    	TimerUpdateUI = new Timer();
    	TimerUpdateUI.schedule(new TimerTask() {
    	       	@Override
    	       	public void run() {
    	       		TimerUpdateUIMethod();
    	       	}
   	   	}, 30, 1000);
    	// initialize timer
  		TimerReadData = new Timer();
  		TimerReadData.schedule(new TimerTask() {
  		    	@Override
  		        public void run() {
		        	TimerReadDataMethod();
    		    }
  		}, 3000, 20000);

    }
    
    void TcpInit() {
    	// //////////////////
    	// TCP client area
    	arrayList = new ArrayList<String>();
    			        
    	mList = (ListView)findViewById(R.id.list);
    	mAdapter = new CustomAdapter(this, arrayList);
    	mList.setAdapter(mAdapter);
    	        
    	new connectTask().execute("");
    }
    
    
    
    
    
    
//    @Override
//	public void onStart(){
//    }	
    
    
    @Override
	public void onStop(){
	    super.onStop();
	    
	    mTcpClient.stopClient();// .run(SERVERIP, SERVERPORT);
	    TimerUpdateUI.cancel();
	    TimerReadData.cancel();
	    finish();
	    System.exit(0);
	}
	
    
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
    	ReadPreferences();
    	InitTimers();
    	TcpInit();            	
    	
    	// //////////////////
     	// set initial styles
     	Typeface ttf_calibri = Typeface.createFromAsset(getAssets(), "fonts/calibri.ttf");
     	//Typeface ttf_calist = Typeface.createFromAsset(getAssets(), "fonts/calist.ttf");

     	TextView TextTempCtrl = (TextView) findViewById(R.id.TextViewTempCtrl);
     	TextView TextMeasuredTemp = (TextView) findViewById(R.id.textViewMeas);
     	TextView TextRefHeat = (TextView) findViewById(R.id. text_heat_ref);
     	TextView TextRefCool = (TextView) findViewById(R.id.text_cool_ref);
     	TextView TextDegC1 = (TextView) findViewById(R.id.text_degree_1);
     	TextView TextDegC2 = (TextView) findViewById(R.id.text_degree_2);
     	Switch switchEnableControl = (Switch) findViewById(R.id.switchEnableControl);
     	Switch switchForceFan = (Switch) findViewById(R.id.switchForceFan);
     	TextMeasuredTemp.setTypeface(ttf_calibri);
     	TextRefHeat.setTypeface(ttf_calibri);
     	TextTempCtrl.setTypeface(ttf_calibri);
     	TextRefCool.setTypeface(ttf_calibri);
     	switchEnableControl.setTypeface(ttf_calibri);
     	switchForceFan.setTypeface(ttf_calibri);
     	TextDegC1.setTypeface(ttf_calibri);
     	TextDegC2.setTypeface(ttf_calibri);
	}
	
	
	public class connectTask extends AsyncTask<String,String,TCPClient> {
		 
        @Override
        protected TCPClient doInBackground(String... message) {
 
        	
            //we create a TCPClient object and
            mTcpClient = new TCPClient(new TCPClient.OnMessageReceived() {
                @Override
                //here the messageReceived method is implemented
                public void messageReceived(String message) {
                    //this method calls the onProgressUpdate
                    publishProgress(message);
                }
            });
            
            //connects to the server
            mTcpClient.run(SERVERIP,SERVERPORT);
            
            return null;
        }
 
        @Override
        protected void onProgressUpdate(String... values) {
            super.onProgressUpdate(values);
 
            String recvStr;
            int intTemp;
            String strTemp;
            float floatTemp;
    	
            Switch switchEnableControl = (Switch) findViewById(R.id.switchEnableControl);
    		Switch switchForceFan = (Switch) findViewById(R.id.switchForceFan);
    		TextView TextCoolRef = (TextView) findViewById(R.id.text_cool_ref);
    		TextView TextHeatRef = (TextView) findViewById(R.id.text_heat_ref);
    		TextView TextTempCtrlStatus = (TextView) findViewById(R.id.TextViewTempCtrl);
    		TextView TextMeasuredTemp = (TextView) findViewById(R.id.textViewMeas);
    		
    		TextView TextDegree1 = (TextView) findViewById(R.id.text_degree_1);
    		TextView TextDegree2 = (TextView) findViewById(R.id.text_degree_2);
    		if (DEGCELCIUS) {
    			TextDegree1.setText("°C");
    			TextDegree2.setText("°C");
    		} else {
    			TextDegree1.setText("F");
    			TextDegree2.setText("F");
    		}
    		
    		
            recvStr = values[0];
            //in the arrayList we add the messaged received from server
            arrayList.add(recvStr);
            // notify the adapter that the data set has changed. This means that new message received
            // from server was added to the list
            mAdapter.notifyDataSetChanged();
            
            if (ReqsFullStatusRequest) {
            	
            	// set status bar
            	strTemp = "";
            	intTemp = recvStr.indexOf("state:", 0) + 6;
            	if (intTemp != -1) {
            		strTemp = recvStr.substring(intTemp, intTemp+1);
            		if (strTemp.matches("1")) TextTempCtrlStatus.setText("Starting up");
            		else if (strTemp.matches("2")) TextTempCtrlStatus.setText("Satisfied");
            		else if (strTemp.matches("3")) TextTempCtrlStatus.setText("Cooling");
            		else if (strTemp.matches("4")) TextTempCtrlStatus.setText("Heating");
            		else if (strTemp.matches("5")) TextTempCtrlStatus.setText("Control disabled");
                	else TextTempCtrlStatus.setText("Answer not expected");
                } else {
                	TextTempCtrlStatus.setText("");
                }
            	
            	// set force fan status
            	strTemp = "";
            	intTemp = recvStr.indexOf("relay3:", 0) + 7;
            	if (intTemp != -1) {
            		strTemp = recvStr.substring(intTemp, intTemp+1);
            		switchForceFan.setEnabled(true);
            		if (strTemp.matches("1")) switchForceFan.setChecked(true);
                	else switchForceFan.setChecked(false);
                } else {
                	switchForceFan.setChecked(false);
                }
            	
            	// set temperature control status
            	strTemp = "";
            	intTemp = recvStr.indexOf("tcenable:", 0) + 9;
            	if (intTemp != -1) {
            		strTemp = recvStr.substring(intTemp, intTemp+1);
            		switchEnableControl.setEnabled(true);
            		if (strTemp.matches("1")) switchEnableControl.setChecked(true);
            		else switchEnableControl.setChecked(false);
            	} else {
            		switchEnableControl.setChecked(false);
            	}
	            
            	// show measured temperature
            	strTemp = "";
            	intTemp = recvStr.indexOf("measured:", 0) + 9;
            	if (intTemp != -1) {
            		strTemp = recvStr.substring(intTemp, intTemp+4);
        	   		if (DEGCELCIUS) {
            	   		TextMeasuredTemp.setText(strTemp + " °C");
            	   	}
	               	else {
	               		try {
	               			floatTemp = Float.parseFloat(strTemp);
	               		} catch(NumberFormatException nfe) {
	               			floatTemp = 0;
	               		}
	               		strTemp = Float.toString(CelctoFar(floatTemp));
	               		strTemp = strTemp.substring(0,4);
	               		TextMeasuredTemp.setText(strTemp + " F");
	               	}
            	   	
	        	} else {
	        		TextMeasuredTemp.setText("");
	        	}
            	
            	// set temperature cool reference
            	strTemp = "";
            	intTemp = recvStr.indexOf("refcool:", 0) + 8;
            	if (intTemp != -1) {
            		strTemp = recvStr.substring(intTemp, intTemp+4);
            	   	if (DEGCELCIUS) {
                	   	TextCoolRef.setText(strTemp);
            		}
            		else {
            			try {
	               			floatTemp = Float.parseFloat(strTemp);
	               		} catch(NumberFormatException nfe) {
	               			floatTemp = 0;
	               		}
	               		strTemp = Float.toString(CelctoFar(floatTemp));
	               		strTemp = strTemp.substring(0,4);
	               		TextCoolRef.setText(strTemp);
            		}
	        	} else {
	        		TextCoolRef.setText("");
	        	}
            	
            	// set temperature heat reference
            	strTemp = "";
            	intTemp = recvStr.indexOf("refheat:", 0) + 8;
            	if (intTemp != -1) {
            		strTemp = recvStr.substring(intTemp, intTemp+4);
            		if (DEGCELCIUS) {
                	   	TextHeatRef.setText(strTemp);
            		}
            		else {
            			try {
	               			floatTemp = Float.parseFloat(strTemp);
	               		} catch(NumberFormatException nfe) {
	               			floatTemp = 0;
	               		}
	               		strTemp = Float.toString(CelctoFar(floatTemp));
	               		strTemp = strTemp.substring(0,4);
	               		TextHeatRef.setText(strTemp);
            		}
           		} else {
           			TextHeatRef.setText("");
           		}
         
            	ReqsFullStatusRequest = false;
            } else if (ReqsCoolRefUpdate) {
            	ReqsCoolRefUpdate = false;
            } else if (ReqsHeatRefUpdate) {
            	ReqsCoolRefUpdate = false;
            } else if (ReqsSetForceFan) {
            	switchForceFan.setEnabled(true);
            	ReqsSetForceFan = false;
            } else if (ReqsSetCtrlEnable) {
            	switchEnableControl.setEnabled(true);
            	ReqsSetCtrlEnable = false;
            } else {
            	// data not expected, do nothing
            }
            	
        }
    }
	
	
	
	
	private void EnableControls(boolean new_state)
	{
		// allow enable and disable all controls over the UI
		Switch switchEnableControl = (Switch) findViewById(R.id.switchEnableControl);
		Switch switchForceFan = (Switch) findViewById(R.id.switchForceFan);
		
		switchEnableControl.setEnabled(new_state);
		switchForceFan.setEnabled(new_state);
		switchEnableControl.setEnabled(new_state);
		switchForceFan.setEnabled(new_state);
	}
	
	private void TimerUpdateUIMethod()
	{
	    //This method is called directly by the timer and runs in the same thread as the timer.
	    //We call the method that will work with the UI through the runOnUiThread method.
	    this.runOnUiThread(TimerUpdateUserInt);	
	}
	
	private void TimerReadDataMethod()
	{
	    //This method is called directly by the timer and runs in the same thread as the timer.
	    //We call the method that will work with the UI through the runOnUiThread method.
	    this.runOnUiThread(TimerReadTcpData);
	}
	
	
	
	private Runnable TimerUpdateUserInt = new Runnable() {
	    public void run() {
	    	
	    	//This method runs in the same thread as the UI.    	       
			//Do something to the UI thread here
	       	TextView TestStatus = (TextView) findViewById(R.id.textViewStatus);
	       	Switch switchForceFan = (Switch) findViewById(R.id.switchForceFan);
			Switch switchTempCtrl = (Switch) findViewById(R.id.switchEnableControl);
			TextView TextRefHeat = (TextView) findViewById(R.id.text_heat_ref);
			TextView TextRefCool = (TextView) findViewById(R.id.text_cool_ref);
				
	    	// check connectivity status
			ConnectivityManager connMgr = (ConnectivityManager) getSystemService(CONNECTIVITY_SERVICE);
			//NetworkInfo networkInfo = connMgr.getActiveNetworkInfo();	// any network connection
			NetworkInfo wifiInfo = connMgr.getNetworkInfo(ConnectivityManager.TYPE_WIFI);  // specific to wifi
			NetworkInfo mobInfo = connMgr.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);  // specific to wifi

			Boolean connectionAvailable = false;
			
			if ((wifiInfo != null && wifiInfo.isConnected()) && (LOCALNETWORK)) {
				connectionAvailable = true;
			}
			else if (mobInfo != null && mobInfo.isConnected()) {
				connectionAvailable = true;
			} 
				
				
			if (connectionAvailable) {
				// enable switches and adjust status text
				EnableControls(true);

				// socket active
				if (mTcpClient != null) {
					if (mTcpClient.isConnected()) {
						TestStatus.setText(R.string.text_status_wifi_tcp_connected);
	
						String NumStr;
					    String FullStatusCommand;
						String message = "";
						
						// answer timeout, clear requests
						if (SystemState == SYSTEMST_WAITINGANS) {
							if (	(ReqsFullStatusRequest == false) & 
									(ReqsSetCtrlEnable == false) & 
									(ReqsSetForceFan == false) & 
									(ReqsCoolRefUpdate == false) & 
									(ReqsHeatRefUpdate == false)) {
								SystemState = SYSTEMST_WAITINGREQS;
								arrayList.add("Answer received");
							} else {
								ReqsFullStatusRequest = false;
								ReqsCoolRefUpdate = false;
								ReqsSetCtrlEnable = false;
								ReqsSetForceFan = false;
								ReqsHeatRefUpdate = false;
								SystemState = SYSTEMST_WAITINGREQS;
						        arrayList.add("Answer timeout");
							}   
						}
						
						// check if there is a request for full status
						else if (ReqsFullStatusRequest) {
							FullStatusCommand = "sf";
							EnableControls(false);
							mTcpClient.sendMessage(FullStatusCommand);
							//refresh the list
						    mAdapter.notifyDataSetChanged();
						    SystemState = SYSTEMST_WAITINGANS;
					        arrayList.add("Waiting answ full status");
						}
						// check request of cool reference change
						else if (ReqsCoolRefUpdate) {
							try	{
								NumStr = TextRefCool.getText().toString();			
							} catch(NumberFormatException nfe) {
								NumStr = DEFAULTREFCOOL;
							}
							if (DEGCELCIUS) {	
							} else {
								float floatTemp;
								try {
									floatTemp = Float.parseFloat(NumStr);
								} catch(NumberFormatException nfe) {
			               			floatTemp = 0;
			               		}
								NumStr = Float.toString(FartoCelc(floatTemp));
							}
							message = "cs02" + NumStr.substring(0, 4);	// limit size of the string in 4 (as expected by system)						    //add the text in the arrayList
					        mTcpClient.sendMessage(message);
					        //refresh the list
					        mAdapter.notifyDataSetChanged();
						    SystemState = SYSTEMST_WAITINGANS;
					        arrayList.add("Waiting answ cool ref");
						}
	
						// check request of cool reference change
						else if (ReqsHeatRefUpdate) {
					        try	{
								NumStr = TextRefHeat.getText().toString();
							} catch(NumberFormatException nfe) {
								NumStr = DEFAULTREFHEAT;
							}
					        if (DEGCELCIUS) {
					        } else {
								float floatTemp;
								try {
									floatTemp = Float.parseFloat(NumStr);
								} catch(NumberFormatException nfe) {
			               			floatTemp = 0;
			               		}
								NumStr = Float.toString(FartoCelc(floatTemp));
							}
				        	message = "cs03" + NumStr.substring(0, 4);	// limit size of the string in 4 (as expected by system)
					        mTcpClient.sendMessage(message);
					        //refresh the list
					        mAdapter.notifyDataSetChanged();
					        SystemState = SYSTEMST_WAITINGANS;
					        arrayList.add("Waiting answ heat ref");
						}
						// check request switch control enable
						else if (ReqsSetCtrlEnable) {
							if (switchTempCtrl.isChecked()) message = "cs151";
							else message = "cs150";
							switchTempCtrl.setEnabled(false);
							mTcpClient.sendMessage(message);
					        arrayList.add("enable ctrl: " + message);
					    	//refresh the list
					        mAdapter.notifyDataSetChanged();
						}	
						// check request switch force fan
						else if (ReqsSetForceFan) {
							if (switchForceFan.isChecked()) message = "is3c";
							else message = "is3o";
							switchForceFan.setEnabled(false);
							mTcpClient.sendMessage(message);
					        arrayList.add("force fan: " + message);
					    	//refresh the list
					        mAdapter.notifyDataSetChanged();
	
						}	
						// no other request, then disconnect socket
						else {
	//TODO						mTcpClient.stopClient();
							SystemState = SYSTEMST_WAITINGREQS;
					        //arrayList.add("Disconnected, waiting reqs");
					}
					// else socket not connected
					} else {
						TestStatus.setText(R.string.text_status_wifi_connected);
						// on any request connect socket
						if (	(ReqsFullStatusRequest) | 
								(ReqsSetCtrlEnable) | 
								(ReqsSetForceFan) | 
								(ReqsCoolRefUpdate) | 
								(ReqsHeatRefUpdate)) {
							SystemState = SYSTEMST_CONNECTING;
							//connects to the server
	//TODO						mTcpClient.run(SERVERIP, SERVERPORT);
					        arrayList.add("Reqs in place, opening socket");
						}	
						else {
							SystemState = SYSTEMST_WAITINGREQS;
						}
					}
				} // tcp client null
			} else {
				// disable switches and adjust status text
				TestStatus.setText(R.string.text_status_wifi_notconnected);
				EnableControls(false);
		        arrayList.add("Wifi not connected");
			}
			
			
			
			
			
	    }
	};
	
	
	

	
	

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		
		return true;
	}
	
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	   // Handle item selection
	   try{
		   switch (item.getItemId()) {
		   case R.id.menu_action_exit:            
			   System.exit(0);
			   return true;        
		   case R.id.action_settings:
			   Intent intent = new Intent(getBaseContext(), SettingsActivity.class);
			   //startActivity(intent);
			   startActivityForResult(intent,RESULT_OK_DIALOG_SETT);

//			   return true;
		   default:
			   return super.onOptionsItemSelected(item);
		   }
	   } catch(Exception e) { 
	   }
	   return true;
	}
	
	
	
	
	
	
	
	

	@Override 
	public void onActivityResult(int requestCode, int resultCode, Intent data) {     
	  super.onActivityResult(requestCode, resultCode, data); 
      if (resultCode == RESULT_OK_DIALOG) { 
    	  float newData = (float)data.getIntExtra("dialog_edit_result", -1) / 10;
    
    	  if (waitingCoolDialogResult) {
	    	  if (newData != -1) {
	    		  TextView TextCoolRef = (TextView) findViewById(R.id.text_cool_ref);
	    		  TextCoolRef.setText(String.valueOf(newData));
	    		  ReqsCoolRefUpdate = true;
	    	  }
    	  }
    	  else if (waitingHeatDialogResult) {
	    	  if (newData != -1) {
	    		  TextView TextHeatRef = (TextView) findViewById(R.id.text_heat_ref);
	    		  TextHeatRef.setText(String.valueOf(newData));
	    		  ReqsHeatRefUpdate = true;
	    	  }    		  
    	  }
      
    	  waitingCoolDialogResult = false;
    	  waitingHeatDialogResult = false;
      }
      else if (resultCode == RESULT_OK_DIALOG_SETT) {
    	  boolean DegC = (boolean)data.getBooleanExtra("setting_deg_c", false);// getIntExtra("dialog_edit_result", -1) / 10;
    	  SERVERIP = data.getStringExtra("setting_new_server");
    	  SERVERPORT = data.getIntExtra("setting_new_port",0);
    	  boolean LocN = (boolean)data.getBooleanExtra("setting_local_net", true);
    	  
    	  // shared preferences
    	  SharedPreferences.Editor editor = getSharedPreferences("connection-settings",MODE_PRIVATE).edit();
    	  editor.putBoolean("deg-celcius",DegC);
    	  editor.putString("server-ip", SERVERIP);
    	  editor.putInt("server-port", SERVERPORT);
    	  editor.putBoolean("server-localnet", LocN);
    	  DEGCELCIUS = DegC;
    	  LOCALNETWORK = LocN;
    	  
    	  editor.commit();
      }
	}

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	public void BtnSendHeatOnClick(View View){		
		TextView TextHeatRef = (TextView) findViewById(R.id.text_heat_ref);
		
		//ReqsHeatRefUpdate = true;
		Intent intent = new Intent(getBaseContext(), DisplayMessageActivity.class);
		if (DEGCELCIUS) {
    		intent.putExtra("dialog_title_str", getString(R.string.dialog_message_heat));
			intent.putExtra("dialog_editn_str", TextHeatRef.getText().toString());
			intent.putExtra("dialog_min_str", 100);
			intent.putExtra("dialog_max_str", 300);
		} else {
			intent.putExtra("dialog_title_str", getString(R.string.dialog_message_heat));
			intent.putExtra("dialog_editn_str", TextHeatRef.getText().toString());
			intent.putExtra("dialog_min_str", 500);
			intent.putExtra("dialog_max_str", 900);
		}
		startActivityForResult(intent,RESULT_OK_DIALOG);
		
		waitingHeatDialogResult = true;
   	}
	
	public void BtnSendCoolOnClick(View View){		
		TextView TextCoolRef = (TextView) findViewById(R.id.text_cool_ref);
	
		//ReqsCoolRefUpdate = true;
		Intent intent = new Intent(getBaseContext(), DisplayMessageActivity.class);
		if (DEGCELCIUS) {
	    	intent.putExtra("dialog_title_str", getString(R.string.dialog_message_cool));
	    	intent.putExtra("dialog_editn_str", TextCoolRef.getText().toString());
	    	intent.putExtra("dialog_min_str", 100);
	    	intent.putExtra("dialog_max_str", 300);
		} else {
	    	intent.putExtra("dialog_title_str", getString(R.string.dialog_message_cool));
	    	intent.putExtra("dialog_editn_str", TextCoolRef.getText().toString());
	    	intent.putExtra("dialog_min_str", 500);
	    	intent.putExtra("dialog_max_str", 900);
		}	
		startActivityForResult(intent,RESULT_OK_DIALOG);
		
		waitingCoolDialogResult = true;
	}
	
	public void SwitchEnableCtrlClick(View View){
		ReqsSetCtrlEnable = true;
	}
	
	public void SwitchForceFanClick(View View){
        ReqsSetForceFan = true;
	}
		
	private Runnable TimerReadTcpData = new Runnable() {
		public void run() {
		    ReqsFullStatusRequest = true;	    
		}
	};
	
}
