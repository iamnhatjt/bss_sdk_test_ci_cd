package com.selex.bssStation.service

import android.os.CountDownTimer
import android.util.Log
import com.selex.bssStation.core.utils.Constants
import com.selex.bssStation.core.utils.TAG

class NoActionCommon {

    var listener: (() -> Unit)? = null

    private val countDownTimer = object : CountDownTimer(60000L, 60000L) {
        override fun onTick(millisUntilFinished: Long) {

        }

        override fun onFinish() {
            Log.d(TAG, "NoActionCommon: Finish no action timer")
            listener?.let { it() }
        }
    }

    companion object {
        val instance = NoActionCommon()
    }

    fun cancelTimer() {
        countDownTimer.cancel()
    }

    fun startNoAction() {
        cancelTimer()
        handleGetInfoBss()
    }

    private fun handleGetInfoBss() {
        countDownTimer.start()
    }
}