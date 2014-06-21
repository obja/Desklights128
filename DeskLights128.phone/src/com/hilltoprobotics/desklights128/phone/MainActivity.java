package com.hilltoprobotics.desklights128.phone;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.PebbleKit.PebbleDataReceiver;
import com.getpebble.android.kit.util.PebbleDictionary;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

public class MainActivity extends Activity {

	private static final UUID APP_UUID = UUID.fromString("07a3fb4a-b4e5-4a98-9a5a-58bedcaf132f");
	private static final int DATA_KEY = 0;
	public TextView x;
	public TextView y;
	public TextView pebbleText;
	public TextView tvIP;
	public Spinner spinner;
	public Spinner spinner2;
	public Button sendBtn;
	public Button sendBtn2;
	public String selected;
	public boolean connected = false;
	public SharedPreferences sharedPrefs;
	public Map <String,String> map;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		x = (TextView) findViewById(R.id.editText1);
		y = (TextView) findViewById(R.id.editText2);
		tvIP = (TextView) findViewById(R.id.textView3);
		pebbleText = (TextView) findViewById(R.id.textView5);
		sendBtn = (Button) findViewById(R.id.button1);
		sendBtn2 = (Button) findViewById(R.id.button2);
		spinner = (Spinner) findViewById(R.id.spinner1);
		spinner2 = (Spinner) findViewById(R.id.spinner2);
		ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,R.array.colors, android.R.layout.simple_spinner_item);
		adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
		sharedPrefs = PreferenceManager.getDefaultSharedPreferences(this);
		
		connected = PebbleKit.isWatchConnected(getApplicationContext());
		if(connected) {
			pebbleText.setText("Pebble connected");
		}
		else {
			pebbleText.setText("Pebble disconnected");
		}
		map =  new HashMap<String,String>();
		map.put("Red","r=255&g=0&b=0");
		map.put("Orange","r=255&g=160&b=0");
		map.put("Yellow","r=255&g=255&b=0");
		map.put("Green","r=0&g=255&b=0");
		map.put("Blue","r=0&g=0&b=255");
		map.put("Purple","r=148&g=0&b=211");
		map.put("Brown","r=139&g=69&b=19");
		map.put("White","r=255&g=255&b=255");
		tvIP.setText("IP Address: " + sharedPrefs.getString("prefIP", "NULL"));
		spinner.setAdapter(adapter);
		spinner2.setAdapter(adapter);
		sendBtn.setOnClickListener(new View.OnClickListener() {
		    @Override
		    public void onClick(View v) {
		        webSend(v);
		    }
		});
		sendBtn2.setOnClickListener(new View.OnClickListener() {
		    @Override
		    public void onClick(View v) {
		        webSend2(v);
		    }
		});
	}
	protected void onResume() {
		super.onResume();
		tvIP.setText("IP Address: " + sharedPrefs.getString("prefIP", "NULL"));
		connected = PebbleKit.isWatchConnected(getApplicationContext());
		if(connected) {
			pebbleText.setText("Pebble connected");
		}
		else {
			pebbleText.setText("Pebbledisconnected");
		}
		
		PebbleKit.startAppOnPebble(getApplicationContext(), APP_UUID);
		PebbleDataReceiver dataHandler;
		dataHandler = new PebbleKit.PebbleDataReceiver(APP_UUID) {
			public void receiveData(Context context, int transactionId, PebbleDictionary data) {
				PebbleKit.sendAckToPebble(context, transactionId);
				int theData = data.getUnsignedInteger(DATA_KEY).intValue();
				
				switch(theData) {
				case 0: {
					webSend3("color?h=FF0000");
					Log.v("pebblestuff","websend 3, red");
					break;
					//send url to make table red
				}
				case 1: {
					webSend3("color?h=FF6600");
					Log.v("pebblestuff","websend 3, orange");
					break;
					//send url to make table red
				}
				case 2: {
					webSend3("color?h=FFFF00");
					Log.v("pebblestuff","websend 3, yellow");
					break;
					//send url to make table red
				}
				case 3: {
					webSend3("color?h=336600");
					Log.v("pebblestuff","websend 3, green");
					break;
					//send url to make table red
				}
				case 4: {
					webSend3("color?h=003333");
					Log.v("pebblestuff","websend 3, blue");
					break;
					//send url to make table red
				}
				case 5: {
					webSend3("color?h=330033");
					Log.v("pebblestuff","websend 3, purple");
					break;
				}
				case 6: {
					webSend3("default?id=1");
					Log.v("pebblestuff","websend 3, rainbow");
					break;
				}
				case 7: {
					webSend3("default?id=2");
					Log.v("pebblestuff","websend 3, random");
					break;
				}
				case 8: {
					webSend3("default?id=3");
					Log.v("pebblestuff","websend 3, cylon");
					break;
				}
				}
			}
		};
		PebbleKit.registerReceivedDataHandler(getApplicationContext(), dataHandler);
	}
	public void webSend(View v) {
		String color = spinner.getItemAtPosition(spinner.getSelectedItemPosition()).toString();
		String url = "http://" + sharedPrefs.getString("prefIP", "NULL") + "/pixel?x=" + x.getText() + "y=" + y.getText() + "&" + map.get(color);
		final ThreadedRequest tReq = new ThreadedRequest(url);
		tReq.start(new Runnable() 
		    {
		        public void run() 
		        {
		        }
		    });
	}
	
	public void webSend2(View v) {
		String color = spinner2.getItemAtPosition(spinner.getSelectedItemPosition()).toString();
		String url = "http://" + sharedPrefs.getString("prefIP", "NULL") + "/color?" + map.get(color);
		final ThreadedRequest tReq = new ThreadedRequest(url);
		tReq.start(new Runnable() 
		    {
		        public void run() 
		        {
		        }
		    });
	}
	
	public void webSend3(String theColor) {
		Log.v("test", theColor);
		String url = "http://" + sharedPrefs.getString("prefIP", "NULL") + "/" + theColor;
		final ThreadedRequest tReq = new ThreadedRequest(url);
		tReq.start(new Runnable() 
		    {
		        public void run() 
		        {
		        }
		    });
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {

		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			Intent i = new Intent(this, Settings.class);
			startActivity(i);
		}
		return super.onOptionsItemSelected(item);
	}
}