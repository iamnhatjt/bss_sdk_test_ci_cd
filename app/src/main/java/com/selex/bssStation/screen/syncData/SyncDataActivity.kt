package com.selex.bssStation.screen.syncData

import android.annotation.SuppressLint
import android.content.Intent
import android.os.Bundle
import android.os.CountDownTimer
import androidx.databinding.DataBindingUtil
import com.selex.bssStation.R
import com.selex.bssStation.databinding.ActivitySyncDataBinding
import com.selex.bssStation.screen.Screen
import com.selex.bssStation.screen.ScreenManager
import com.selex.bssStation.screen.base.BaseActivity
import com.selex.bssStation.screen.standBy.ScreenStandbyActivity
import com.selex.bssStation.service.BackgroundService

class SyncDataActivity : BaseActivity() {

    private lateinit var _binding: ActivitySyncDataBinding

    @SuppressLint("SetTextI18n")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        _binding = DataBindingUtil.setContentView(this, R.layout.activity_sync_data)
        startServiceApp()

        val time = 60000L
        object : CountDownTimer(time, time / 100) {
            override fun onTick(currentTime: Long) {
                val percent = ((time - currentTime).toDouble() / time * 100).toInt()
                _binding.pbPercentSyncData.progress = percent
                _binding.tvPercent.text = "$percent%"
            }
            override fun onFinish() {
                _binding.pbPercentSyncData.progress = 100
                _binding.tvPercent.text = "100%"
                BackgroundService.isNeedSyncData = true
                ScreenStandbyActivity.startActivity(this@SyncDataActivity)
                BackgroundService.syncData()
                finish()
            }
        }.start()
    }

    override fun onResume() {
        super.onResume()
        ScreenManager.currentScreen = Screen.SYNC_DATA
    }

    private fun startServiceApp() {
        startService(Intent(this, BackgroundService::class.java))
    }

}
