package com.hilltoprobotics.desklights128.phone;

import java.util.HashMap;
import java.util.Map;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

public class MainActivity extends Activity {

	public TextView x;
	public TextView y;
	public TextView tvIP;
	public Spinner spinner;
	public Spinner spinner2;
	public Button sendBtn;
	public Button sendBtn2;
	public String selected;
	public SharedPreferences sharedPrefs;
	public Map <String,String> map;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		x = (TextView) findViewById(R.id.editText1);
		y = (TextView) findViewById(R.id.editText2);
		tvIP = (TextView) findViewById(R.id.textView3);
		sendBtn = (Button) findViewById(R.id.button1);
		sendBtn2 = (Button) findViewById(R.id.button2);
		spinner = (Spinner) findViewById(R.id.spinner1);
		spinner2 = (Spinner) findViewById(R.id.spinner2);
		ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,R.array.colors, android.R.layout.simple_spinner_item);
		adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
		sharedPrefs = PreferenceManager.getDefaultSharedPreferences(this);
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
	}
	public void webSend(View v) {
		String color = spinner.getItemAtPosition(spinner.getSelectedItemPosition()).toString();
		String url = "http://" + sharedPrefs.getString("prefIP", "NULL") + "pixel?x=" + x.getText() + "y=" + y.getText() + "&" + map.get(color);
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