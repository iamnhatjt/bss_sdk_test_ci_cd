package com.selex.bssStation.core.utils
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.os.CountDownTimer
import android.util.Log
import com.selex.bssStation.screen.Screen
import com.selex.bssStation.screen.ScreenManager

class RestartStationReceiver: BroadcastReceiver() {

    override fun onReceive(context: Context, intent: Intent) {
        object : CountDownTimer(3000, 3000) {
            override fun onTick(p0: Long) {}

            override fun onFinish() {
                if (ScreenManager.currentScreen == Screen.STAND_BY) {
                    Log.d(TAG, "Request admin restart station")
                    context.sendBroadcast(Intent(ACTION_REBOOT_REQUEST))
                } else {
                    this.start()
                }
            }
        }.start()
    }

}
