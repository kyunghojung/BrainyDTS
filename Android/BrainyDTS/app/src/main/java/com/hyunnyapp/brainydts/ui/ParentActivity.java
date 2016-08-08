package com.hyunnyapp.brainydts.ui;

import android.app.Activity;
import android.os.Bundle;
import android.view.KeyEvent;
import android.widget.Toast;

import com.hyunnyapp.brainylivingroom.R;

public class ParentActivity extends Activity
{

	private ActivityManager am = ActivityManager.getInstance();

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		am.addActivity(this);
	}

	@Override
	protected void onDestroy()
	{
		super.onDestroy();
		am.removeActivity(this);
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.ECLAIR
				&& keyCode == KeyEvent.KEYCODE_BACK
				&& event.getRepeatCount() == 0)
		{
			onBackPressed();
		}

		return super.onKeyDown(keyCode, event);
	}

	private final long	FINSH_INTERVAL_TIME    = 2000;
	private long		backPressedTime        = 0;

	@Override
	public void onBackPressed()
	{
		long tempTime = System.currentTimeMillis();
		long intervalTime = tempTime - backPressedTime;

		if (intervalTime >= 0 && intervalTime <= FINSH_INTERVAL_TIME)
		{
			super.onBackPressed();
			am.finishAllActivity();
		}
		else
		{
			backPressedTime = tempTime;
			Toast.makeText(getApplicationContext(), getString(R.string.back_button_press_and_exit), Toast.LENGTH_SHORT).show();
		}
		return;
	}
}
