package com.hilltoprobotics.desklights128.glass;

import java.io.IOException;

import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.util.EntityUtils;

import android.os.Handler;
import android.util.Log;

public class ThreadedRequest
{
    private String url;
    private Handler mHandler;
    private Runnable pRunnable;
    private int statusCode;

    public ThreadedRequest(String newUrl)
    {
        url = newUrl;
        mHandler = new Handler();
    }

    public void start(Runnable newRun)
    {
        pRunnable = newRun;
        processRequest.start();
    }

    private Thread processRequest = new Thread()
    {
        public void run()
        {
            //Do you request here...
        	Log.v("t", "Web Send!");
        	String str = null;
    			try
    	    	{
    	    		HttpClient hc = new DefaultHttpClient();
    	    		HttpPost post = new HttpPost(url);

    	    		HttpResponse rp = hc.execute(post);

    	    		if(rp.getStatusLine().getStatusCode() == HttpStatus.SC_OK)
    	    		{
    	    			str = EntityUtils.toString(rp.getEntity());
    	    		}
    	    	}catch(IOException e){
    	    		e.printStackTrace();
    	    	} 
            if (pRunnable == null || mHandler == null) return;
            mHandler.post(pRunnable);
        }
    };
}