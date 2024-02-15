package com.selex.bssStation.core.utils
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.util.Log

import org.greenrobot.eventbus.EventBus


class NetworkChangeReceiver : BroadcastReceiver() {

    override fun onReceive(context: Context, intent: Intent) {
        val isConnected = NetWorkUtil.isNetworkConnected(context)
        EventBus.getDefault().post(NetworkChangeEvent(isConnected))
    }

}