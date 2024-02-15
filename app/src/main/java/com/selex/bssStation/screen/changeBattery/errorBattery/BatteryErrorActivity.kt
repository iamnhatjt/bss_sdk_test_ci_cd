package com.selex.bssStation.screen.changeBattery.errorBattery

import android.annotation.SuppressLint
import android.app.Activity
import android.app.AlertDialog
import android.content.Context
import android.content.DialogInterface
import android.content.Intent
import android.os.Bundle
import android.os.CountDownTimer
import android.util.Log
import android.view.View
import android.view.WindowManager
import androidx.databinding.DataBindingUtil
import com.selex.bssStation.R
import com.selex.bssStation.core.utility.BottomDialogFragment
import com.selex.bssStation.core.utility.clickWithThrottle
import com.selex.bssStation.core.utility.showSnackbar
import com.selex.bssStation.core.utils.Constants
import com.selex.bssStation.core.utils.TAG
import com.selex.bssStation.core.utils.logout
import com.selex.bssStation.databinding.ActivityStationCannotReadBatterySecondBinding
import com.selex.bssStation.model.error.ErrorResponse
import com.selex.bssStation.screen.Screen
import com.selex.bssStation.screen.ScreenManager
import com.selex.bssStation.screen.base.BaseActivity
import com.selex.bssStation.screen.changeBattery.validateOldBattery.ValidateOldBatteryActivity
import com.selex.bssStation.screen.home.MainActivity
import com.selex.bssStation.service.*
import kotlinx.coroutines.*
import setIndexCabinetToBE


class BatteryErrorActivity : BaseActivity() {

    private lateinit var _binding: ActivityStationCannotReadBatterySecondBinding
    private var oldCabinetId: Int = -1
    private var transactionId: Int = -1
    private var error: ErrorResponse? = null
    private var countDownTimer: CountDownTimer? = null
    private var countDownTimerNoAction: CountDownTimer? =  object : CountDownTimer(60000L, 1000) {
        override fun onTick(millisUntilFinished: Long) {
            val timer = 60 - millisUntilFinished/1000
            if (timer.toInt() > 3) {
                _binding.btnBack.isEnabled = true
                _binding.btnBack.isClickable = true
                _binding.btnRetry.isEnabled = true
                _binding.btnRetry.isClickable = true
            }
            val state = getDoorStateFromJNI(oldCabinetId)
            if (timer.toInt() == 30 && state != 0) {
                if(ScreenManager.currentScreen == Screen.BATTERY_ERROR) {
                    _binding.bottomDialogFragment.getFragment<BottomDialogFragment>()
                        .show(title = getString(R.string.please_close_station), message = getString(R.string.error_cabinet), timerShow = 20000L)
                }
            }
        }
        override fun onFinish() {
            Log.d(TAG, "${javaClass.simpleName}: Time out countDownTimerNoAction(), Logging out ...")
            logout(this@BatteryErrorActivity)
        }
    }
    companion object {
        fun startActivity(activity: Activity, bundle: Bundle? = null, oldCabinetId: Int, transactionId: Int?, error: ErrorResponse ? = null) {

            val intent = Intent(activity, BatteryErrorActivity::class.java)
            if (bundle!= null) {
                intent.putExtras(bundle)
            }
            if (transactionId != null) {
                intent.putExtra("transactionId", transactionId);
            }
            intent.putExtra("ERROR",error)
            intent.putExtra("oldCabinetId", oldCabinetId);
            activity.startActivity(intent)
        }
    }
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.i(TAG, "BatteryErrorActivity: OnCreate")

        _binding = DataBindingUtil.setContentView(this, R.layout.activity_station_cannot_read_battery_second)
        oldCabinetId = intent.getIntExtra("oldCabinetId", -1)
        if(intent.hasExtra("transactionId")){
            transactionId = intent.getIntExtra("transactionId", -1)
        }
        if(intent.hasExtra("ERROR")){
            error = intent.getSerializableExtra("ERROR") as? ErrorResponse
        }
        checkSOC()
        handierNoAction()
        _binding.btnBack.setOnClickListener {
            Log.d(TAG, "${javaClass.simpleName}: Click back")
            countDownTimerNoAction?.cancel()
//            if (state == 0) {
                // cua dong
                showDefaultDialog(this)
//            } else {
//                // cua mo, show dialog bat dong
//                showSnackbar(_binding.root, message = resources.getText(R.string.please_close_station).toString())
//                handierNoAction()
//            }
        }
        _binding.btnRetry.clickWithThrottle{
            Log.d(TAG, "${javaClass.simpleName}: Click retry")
            val state = getDoorStateFromJNI(oldCabinetId)
            countDownTimerNoAction?.cancel()
            if (state == 0) {
                // cua dong
                ValidateOldBatteryActivity.startActivity(this, oldCabinetId = oldCabinetId, transactionId = transactionId)
                finish()
            } else {
                // cua mo, show dialog bat dong
                showSnackbar(_binding.root, message = resources.getText(R.string.please_close_station).toString())
                handierNoAction()
            }
        }
    }

    override fun onResume() {
        super.onResume()
        Log.i(TAG, "BatteryErrorActivity: OnResume")
        ScreenManager.currentScreen = Screen.BATTERY_ERROR
    }

    override fun onPause() {
        Log.i(TAG, "BatteryErrorActivity: OnPause")
        super.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.i(TAG, "BatteryErrorActivity: OnDestroy")
    }

    override fun finish() {
        super.finish()
        Log.i(TAG, "BatteryErrorActivity: finish")
        countDownTimer?.cancel()
        countDownTimerNoAction?.cancel()
        overridePendingTransition(R.anim.left_in, R.anim.right_out)
    }

    private fun showDefaultDialog(context: Context) {
        handierNoAction()
        val alertDialog = AlertDialog.Builder(context)
        alertDialog.apply {
            setTitle(getString(R.string.dialog_warning_title))
            setMessage(getString(R.string.dialog_warning_message))
            setPositiveButton(R.string.dialog_accept) { _: DialogInterface?, _: Int ->
                Log.d(TAG, "${javaClass.simpleName}: Dialog: Accept back to Home Screen")
                giveBackOldPin()
                BackgroundService.isNeedSyncData = true
                countDownTimerNoAction?.cancel()
                onBackPressedDispatcher.onBackPressed()
            }
            setNegativeButton(R.string.dialog_cancel) { _, _ ->
                Log.d(TAG, "${javaClass.simpleName}: Dialog: Cancel back to Home Screen")
                handierNoAction()
            }
            setNeutralButton(R.string.dialog_opencabin) { _, _ ->
                Log.d(TAG, "${javaClass.simpleName}: Dialog: Open Cabinet")
                handierNoAction()
                giveBackOldPin()
            }

        }
        val dialog = alertDialog.create()
        dialog.show()
        dialog.window!!.decorView.systemUiVisibility = (
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                        or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION)
        dialog.window!!.clearFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE)
    }


    private fun handierNoAction() {
        _binding.btnBack.isEnabled = false
        _binding.btnBack.isClickable = false
        _binding.btnRetry.isEnabled = false
        _binding.btnRetry.isClickable = false
        countDownTimerNoAction?.cancel()
        countDownTimerNoAction?.start()
    }

    private  fun giveBackOldPin() {
        if (oldCabinetId != -1) {
            //Handler().postDelayed({
            Log.d(TAG, "${javaClass.simpleName}: Give Old Battery in cabinet: ${setIndexCabinetToBE(oldCabinetId)}")
            setDoorOpen(oldCabinetId)
            // }, 2000L)
        }
    }

    private fun checkSOC() {
        val soc = getSocBatteryFromJNI(oldCabinetId)
        Log.d(TAG, "${javaClass.simpleName}: Check SOC in cabinet ${setIndexCabinetToBE(oldCabinetId)} is: $soc")
        val serial: String? = getSerialBattery(oldCabinetId)
        Log.d(TAG, "${javaClass.simpleName}: Battery Serial in cabinet ${setIndexCabinetToBE(oldCabinetId)} is: $serial")
        if (serial?.isNotEmpty() == true) {
            countDownTimer = object : CountDownTimer(5000L, 1000) {
                override fun onTick(millisUntilFinished: Long) {
                    val soc = getSocBatteryFromJNI(oldCabinetId)
                    if (soc > 0) {
                        countDownTimer?.cancel()
                        systemHasCheckedBattery()
                        giveBackOldPin()
                    }
                }
                override fun onFinish() {
                    systemHasCheckedBattery()
                    giveBackOldPin()
                }
            }.start()
        } else {
            systemHasCheckedBattery()
            giveBackOldPin()
        }
        if (error != null) {
            _binding.tvPercentBattery.visibility = View.VISIBLE
            _binding.tvSerialBattery.visibility = View.VISIBLE
            _binding.tvStationCanNotReadBattery.text = error!!.message
            _binding.tvPleasePutBatteryCorrect.text = resources.getString(R.string.please_check_again_or_call_support)
            _binding.tvOverlay.visibility = View.VISIBLE
            _binding.layoutBottom.visibility = View.VISIBLE
        } else {
            _binding.tvStationCanNotReadBattery.text = resources.getString(R.string.station_can_not_read_battery)
            _binding.tvPleasePutBatteryCorrect.text = resources.getString(R.string.please_put_battery_correct_position_and_again)
            _binding.tvPercentBattery.visibility = View.GONE
            _binding.tvSerialBattery.visibility = View.GONE
            _binding.tvOverlay.visibility = View.VISIBLE
            _binding.layoutBottom.visibility = View.VISIBLE
        }

    }

    @SuppressLint("SetTextI18n")
    private fun systemHasCheckedBattery() {
        if (error != null && oldCabinetId != -1) {
            val serial: String? = getSerialBattery(oldCabinetId)
            val soc = getSocBatteryFromJNI(oldCabinetId)
            Log.d(TAG, "${javaClass.simpleName}: SN: $serial and SOC: $soc")
            if (soc < 20 ) {
                _binding.imgCanNotReadBattery.setImageResource(R.drawable.img_battery_1_20)
            } else if (soc in 20..50) {
                _binding.imgCanNotReadBattery.setImageResource(R.drawable.img_battery_20_50)
            } else if (soc in 50..70) {
                _binding.imgCanNotReadBattery.setImageResource(R.drawable.img_battery_50_70)
            } else if (soc >= 70) {
                _binding.imgCanNotReadBattery.setImageResource(R.drawable.img_battery_full)
            }
            _binding.tvSerialBattery.text = serial
            _binding.tvPercentBattery.text = "$soc%"
        }
    }
}