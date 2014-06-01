package com.hilltoprobotics.desklights128.glass;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import com.google.android.glass.app.Card;

import android.app.Activity;
import android.os.Bundle;
import android.speech.RecognizerIntent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends Activity {

public TextView tv;
public Map <String,String> map;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		Card card1 = new Card(this);
		View card1View = card1.getView();
		card1.setText("Launch this using the keyword instead");
		setContentView(card1View);
		tv = (TextView) findViewById(R.id.tv);
		map =  new HashMap<String,String>();
		map.put("red","r=255&g=0&b=0");
		map.put("orange","r=255&g=160&b=0");
		map.put("yellow","r=255&g=255&b=0");
		map.put("green","r=0&g=255&b=0");
		map.put("blue","r=0&g=0&b=255");
		map.put("purple","r=148&g=0&b=211");
		map.put("brown","r=139&g=69&b=19");
		map.put("white","r=255&g=255&b=255");
		map.put("off","r=0&g=0&b=0");
	}

	protected void onResume() {
		super.onResume();
		Card card1 = new Card(this);
		View card1View = card1.getView();
		ArrayList<String> voiceResults = getIntent().getExtras().getStringArrayList(RecognizerIntent.EXTRA_RESULTS);
		String spokenText = voiceResults.get(0);
		card1.setText("Setting the table to " + spokenText);
		setContentView(card1View);
		String url = "http://192.168.1.130:7080/index.php" + "?color=" + map.get(spokenText);
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
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

}
