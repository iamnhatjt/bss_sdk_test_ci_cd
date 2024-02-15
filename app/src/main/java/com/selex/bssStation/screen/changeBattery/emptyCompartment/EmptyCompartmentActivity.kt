package com.selex.bssStation.screen.changeBattery.emptyCompartment

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.os.CountDownTimer
import android.util.Log
import androidx.databinding.DataBindingUtil
import androidx.recyclerview.widget.GridLayoutManager
import com.selex.bssStation.R
import com.selex.bssStation.core.utility.BottomDialogFragment
import com.selex.bssStation.core.utils.TAG
import com.selex.bssStation.core.utils.logout
import com.selex.bssStation.databinding.ActivityChangeBatterySecondBinding
import com.selex.bssStation.screen.Screen
import com.selex.bssStation.screen.ScreenManager
import com.selex.bssStation.screen.base.BaseActivity
import com.selex.bssStation.screen.changeBattery.validateOldBattery.ValidateOldBatteryActivity
import com.selex.bssStation.service.getDoorStateFromJNI
import com.selex.bssStation.service.setDoorOpen
import dagger.hilt.android.AndroidEntryPoint
import getIndexFromBE
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import setIndexCabinetToBE

@AndroidEntryPoint
class EmptyCompartmentActivity : BaseActivity() {
    private lateinit var _binding: ActivityChangeBatterySecondBinding
    private  var listCompartmentAdapter: ListCompartmentAdapter? = null
    private var availableCabinetId = -1
    private var transactionId = -1
    private var countDownTimer: CountDownTimer? = null
    private var countDownTimerOpenDoor: CountDownTimer? = null
    companion object {
        fun startActivity(activity: Activity, bundle: Bundle? = null, availableCabinetId: Int, transactionId: Int ) {
            val intent = Intent(activity, EmptyCompartmentActivity::class.java)
            if (bundle != null) {
                intent.putExtras(bundle)
            }
            intent.putExtra("availableCabinetId", availableCabinetId);
            intent.putExtra("transactionId", transactionId);
            intent.flags = Intent.FLAG_ACTIVITY_SINGLE_TOP or Intent.FLAG_ACTIVITY_CLEAR_TOP

            activity.startActivity(intent)
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.i(TAG, "EmptyCompartmentActivity: OnCreate")
        _binding = DataBindingUtil.setContentView(this, R.layout.activity_change_battery_second)
        availableCabinetId = getIndexFromBE(intent.getIntExtra("availableCabinetId", -1))

        transactionId = intent.getIntExtra("transactionId", -1)
        _binding.tvPositionOpen.text = "Đã mở cửa khoang số ${setIndexCabinetToBE(availableCabinetId)}"
        showListBatteryCompartment()
        handleOpenCabinet()
        handierOpenPin()
    }
    override fun onResume() {
        super.onResume()
        Log.i(TAG, "EmptyCompartmentActivity: OnResume")
        ScreenManager.currentScreen = Screen.EMPTY_COMPARTMENT
    }

    override fun onPause() {
        Log.i(TAG, "EmptyCompartmentActivity: OnPause")
        super.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.i(TAG, "EmptyCompartmentActivity: OnDestroy")
        countDownTimer?.cancel()
        countDownTimerOpenDoor?.cancel()
    }

    override fun finish() {
        super.finish()
        Log.i(TAG, "EmptyCompartmentActivity: finish")
        countDownTimer?.cancel()
        countDownTimerOpenDoor?.cancel()
        listCompartmentAdapter?.cancelTimerAnimation()
        overridePendingTransition(R.anim.left_in, R.anim.right_out)
    }

    private fun handierOpenPin() {
        countDownTimerOpenDoor?.cancel()
        countDownTimerOpenDoor = object : CountDownTimer(10000L, 1000) {
            override fun onTick(millisUntilFinished: Long) {
                val state = getDoorStateFromJNI(availableCabinetId)
                if (state == 1) {
                    countDownTimerOpenDoor?.cancel()
                    Log.d(TAG, "${javaClass.simpleName}: handierValidate() from onTick")
                    handierValidate()
                }
            }
            override fun onFinish() {
                val state = getDoorStateFromJNI(availableCabinetId)
                if (state == 0) {
                    Log.d(TAG, "${javaClass.simpleName}: handierValidate() from onFinish")
                    handierValidate()
                } else {
                    Log.d(TAG, "${javaClass.simpleName}: CABINET BLOCKED")
                    handierValidate()
                }
            }
        }.start()
    }

    private fun handierValidate() {
        Log.d(TAG, "${javaClass.simpleName}: OPENED CABINET ${setIndexCabinetToBE(availableCabinetId)}")
        countDownTimer = object : CountDownTimer(60000L, 1000) {
            override fun onTick(millisUntilFinished: Long) {
                val timer = millisUntilFinished/1000

                val state = getDoorStateFromJNI(availableCabinetId)
                if (state == 0 ) {
                    countDownTimer?.cancel()
                    ValidateOldBatteryActivity.startActivity(
                        this@EmptyCompartmentActivity,
                        oldCabinetId = availableCabinetId,
                        transactionId = transactionId
                    )
                    finish()
                }
                if (timer.toInt() == 30 && state != 0) {
                    if(ScreenManager.currentScreen == Screen.EMPTY_COMPARTMENT) {
                        _binding.bottomDialogFragment.getFragment<BottomDialogFragment>().show(title = getString(R.string.please_close_station), message = getString(R.string.error_cabinet), timerShow  = 20000L)
                    }
                }
            }
            override fun onFinish() {
                Log.d(TAG, "${javaClass.simpleName}: Time out handierValidate(), Logging out ...")
                logout(this@EmptyCompartmentActivity)
            }
        }.start()
    }

    private fun showListBatteryCompartment() {
        _binding.rcyBatteryCompartment.apply {
            layoutManager = GridLayoutManager(context, 4, GridLayoutManager.VERTICAL, false)
            listCompartmentAdapter = ListCompartmentAdapter(animatedIndex = setIndexCabinetToBE(availableCabinetId))
            adapter = listCompartmentAdapter
        }
    }
    private fun handleOpenCabinet() {
        Log.d(TAG, "${javaClass.simpleName}: Open cabinet empty ")
        GlobalScope.launch {
            val value =  setDoorOpen(availableCabinetId)
            Log.d(TAG, "${javaClass.simpleName}: Open cabinet empty $value")
            if (value < 0) {
                Log.d(TAG, "${javaClass.simpleName}: Open cabinet empty failed")
//                setDoorOpenFromJNI(availableCabinetId)
            }
        }
    }
}