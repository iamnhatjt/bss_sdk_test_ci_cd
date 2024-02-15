package com.selex.bssStation.screen.changeBattery.resultSuccess

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.os.CountDownTimer
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.View
import androidx.activity.viewModels
import androidx.databinding.DataBindingUtil
import com.selex.bssStation.R
import com.selex.bssStation.core.utility.BottomDialogFragment
import com.selex.bssStation.core.utility.clickWithThrottle
import com.selex.bssStation.core.utility.showSnackbar
import com.selex.bssStation.core.utils.Constants
import com.selex.bssStation.core.utils.TAG
import com.selex.bssStation.core.utils.NetworkResult
import com.selex.bssStation.core.utils.logout
import com.selex.bssStation.databinding.ActivityChangeBatterySuccessBinding
import com.selex.bssStation.model.confirmChangeBattery.ConfirmChangeBatteryModel
import com.selex.bssStation.screen.Screen
import com.selex.bssStation.screen.ScreenManager
import com.selex.bssStation.screen.base.BaseActivity
import com.selex.bssStation.screen.changeBattery.emptyCompartment.EmptyCompartmentActivity
import com.selex.bssStation.screen.changeBattery.validateOldBattery.ValidateOldBatteryActivity
import com.selex.bssStation.screen.home.MainActivity
import com.selex.bssStation.service.*
import getIndexFromBE
import org.greenrobot.eventbus.EventBus
import org.greenrobot.eventbus.Subscribe
import org.greenrobot.eventbus.ThreadMode
import org.json.JSONObject

class ResultSucessActivity : BaseActivity() {

    private lateinit var _binding: ActivityChangeBatterySuccessBinding
    private val viewModel: ResultSucessViewModel by viewModels()
    private var countDownTimer: CountDownTimer? = null
    private var model: ConfirmChangeBatteryModel? = null
    private var isContinue = false
    private var isFinish = false
    private var haveTransaction = false
    companion object {
        private const val CONFIRM_PIN_RESULT = "CONFIRM_PIN_RESULT"
        fun startActivity(
            activity: Activity,
            bundle: Bundle? = null,
            model: ConfirmChangeBatteryModel
        ) {
            val intent = Intent(activity, ResultSucessActivity::class.java)
            if (bundle != null) {
                intent.putExtras(bundle)
            }
            intent.putExtra(CONFIRM_PIN_RESULT, model)
            intent.flags = Intent.FLAG_ACTIVITY_SINGLE_TOP or Intent.FLAG_ACTIVITY_CLEAR_TOP
            activity.startActivity(intent)
        }
    }

    @SuppressLint("SuspiciousIndentation")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.i(TAG, "ResultSucessActivity: onCreate")
        _binding = DataBindingUtil.setContentView(this, R.layout.activity_change_battery_success)

        if (intent.hasExtra(CONFIRM_PIN_RESULT)) {
            model = intent.getSerializableExtra(CONFIRM_PIN_RESULT) as? ConfirmChangeBatteryModel
        }

        configData()

        Handler().postDelayed({
            countDownTimerLogout()
        }, 3000)

        _binding.btnLogout.setOnClickListener {
            Log.d(TAG, "ResultSucessActivity: Click logout")
            model?.newCab?.let {
                val state = getDoorStateFromJNI(getIndexFromBE(it))
                if (state == 0) {
                    // cua dong
                    Log.d(TAG, "ResultSucessActivity: Click logout onFinish")
                    countDownTimer?.onFinish()
                } else {
                    // cua mo, show dialog bat dong
                    showSnackbar(_binding.root, message = getString(R.string.please_close_station))
                }
            }
        }

        _binding.btnContinue.clickWithThrottle{
            Log.d(TAG, "ResultSucessActivity: Click continue")
            model?.newCab?.let {
                val state = getDoorStateFromJNI(getIndexFromBE(it))
                if (state == 0) {
                    // cua dong
                    isContinue = true
                    Log.d(TAG, "ResultSucessActivity: Click continue continueSwap")
                    continueSwap()
                } else {
                    // cua mo, show dialog bat dong
                    showSnackbar(_binding.root, message = getString(R.string.please_close_station))
                }
            }
        }

        viewModel.responseEmptyCabinet.observe(this) { result ->
            Log.i(TAG, "responseEmptyCabinet: $result")
            _binding.apply {
                btnContinue.isEnabled = true
                btnContinue.isClickable = true
                btnLogout.isEnabled = true
                btnLogout.isClickable = true

                progressBarContinue.visibility = View.GONE

                imageViewContinue.visibility = View.VISIBLE
                textViewContinue.visibility = View.VISIBLE
            }
            if (result.data != null) {
                haveTransaction = true
                Log.i(TAG, "responseEmptyCabinet: $isFinish")
                if (!isFinish) {
                    model?.newCab?.let {
                        val serial = getSerialBattery(getIndexFromBE(it)) ?: ""
                        if (isContinue && serial.isEmpty()) {
                            result.data.availableCabinetId?.let { availableCabinetId ->
                                result.data.transactionId?.let { transactionId ->
                                    finish()
                                    EmptyCompartmentActivity.startActivity(
                                        activity = this,
                                        availableCabinetId = availableCabinetId,
                                        transactionId = transactionId
                                    )
                                }
                            }
                        } else {
                            result.data.availableCabinetId?.let { availableCabinetId ->
                                finish()
                                result.data.transactionId?.let { transactionId ->
                                    ValidateOldBatteryActivity.startActivity(
                                        activity = this,
                                        oldCabinetId = getIndexFromBE(availableCabinetId),
                                        transactionId = transactionId
                                    )

                                }
                            }
                        }
                    }
                }
            } else {
                result.error?.message?.let { it1 ->
                    _binding.bottomDialogFragment.getFragment<BottomDialogFragment>()
                        .show(message = it1, title = getString(R.string.dialog_error))
                }
                // tra lai pin cu
                model?.newCab?.let {
                    Log.d(TAG, "ResultSucessActivity: Give Old Pin")
                  val result =  setDoorOpen(getIndexFromBE(it))
                    if (result < 0) {
                        Log.d(TAG, "ResultSucessActivity: Give Old Pin retry error")
                        //setDoorOpenFromJNI(getIndexFromBE(it))
                    }
                }
                countDownTimer?.start()
            }
        }

    }

    override fun finish() {
        isFinish = true
        countDownTimer?.cancel()
        super.finish()
        Log.i(TAG, "ResultSucessActivity: finish")

        overridePendingTransition(R.anim.left_in, R.anim.right_out)
    }

    override fun onStart() {
        super.onStart()
        Log.i(TAG, "ResultSucessActivity: onStart")
    }

    override fun onStop() {
        super.onStop()
        Log.i(TAG, "ResultSucessActivity: onStop")
    }

    override fun onResume() {
        super.onResume()
        Log.i(TAG, "ResultSucessActivity: OnResume")
        ScreenManager.currentScreen = Screen.RESULT_SUCCESS
    }

    override fun onPause() {
        Log.i(TAG, "ResultSucessActivity: OnPause")
        super.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.i(TAG, "ResultSucessActivity: OnDestroy")
    }


    private fun logout() {
        logout(this, needSyncData = false)
    }

    private fun countDownTimerLogout() {
        _binding.btnContinue.isEnabled = true
        _binding.btnContinue.isClickable = true
        _binding.btnLogout.isEnabled = true
        _binding.btnLogout.isClickable = true
        countDownTimer = object :
            CountDownTimer(Constants.COUNTDOWN_TIMER_LOGOUT_WHEN_CHANGE_BATTERY_SUCCESS, 1000) {
            @SuppressLint("SetTextI18n")
            override fun onTick(millisUntilFinished: Long) {
                val serial = model?.newCab?.let { getSerialBattery(getIndexFromBE(it)) } ?: ""
                val state = model?.newCab?.let { getDoorStateFromJNI(getIndexFromBE(it)) }

                _binding.tvTimeRemaining.text = "${millisUntilFinished / 1000}s"
                _binding.pbTimeRemaining.progress = (millisUntilFinished / 2000).toInt()
                Log.d(
                    TAG,
                    "ResultSucessActivity: changeNextBattery from countDownTimerLogout() ${millisUntilFinished / 1000}  $serial"
                )
                if (state == 0 && serial.isNotEmpty() && !haveTransaction) {
                    var obj = JSONObject(BackgroundService.resultBatteryConnected)
                    var id = obj["id"]
                    Log.d(TAG, "ResultSucessActivity timer: BATTERY_CONNECTED $obj ")
                    if (id == model?.newCab?.let { getIndexFromBE(it) }) {
                        isContinue = false
                        Log.d(
                            TAG,
                            "ResultSucessActivity: changeNextBattery from countDownTimerLogout()ddddddddd"
                        )
                        countDownTimer?.cancel()
                        changeNextBattery()
                    }
                }
            }

            override fun onFinish() {
                Log.d(TAG, "ResultSucessActivity: Logout in Result success...")
                // Remove token and back to StandByScreen
                if (ScreenManager.currentScreen == Screen.RESULT_SUCCESS) {
                    logout()
                }
            }
        }.start()
    }

    private fun changeNextBattery() {
        _binding.apply {
            btnContinue.isEnabled = false
            btnContinue.isClickable = false
            btnLogout.isEnabled = false
            btnLogout.isClickable = false

            imageViewContinue.visibility = View.GONE
            textViewContinue.visibility = View.GONE

            progressBarContinue.visibility = View.VISIBLE
        }
        countDownTimer?.cancel()
        model?.newCab?.let { it ->
            val serial: String? = model?.newCab?.let { getSerialBattery(getIndexFromBE(it)) }
            val state = model?.newCab?.let { getDoorStateFromJNI(getIndexFromBE(it)) }
            if ((state == 0 && serial?.isNotEmpty() == true)) {
                viewModel.createNewTransactionId(it)
            } else {
                _binding.btnLogout.isEnabled = true
                _binding.btnLogout.isClickable = true
                countDownTimer?.start()
            }
        }
    }

    private fun continueSwap() {
        _binding.apply {
            btnContinue.isEnabled = false
            btnContinue.isClickable = false
            btnLogout.isEnabled = false
            btnLogout.isClickable = false

            imageViewContinue.visibility = View.GONE
            textViewContinue.visibility = View.GONE

            progressBarContinue.visibility = View.VISIBLE
        }
        countDownTimer?.cancel()
        model?.newCab?.let {
            viewModel.createNewTransactionId(it)
        }
    }


    @SuppressLint("SetTextI18n")
    private fun configData() {
        model?.run {
            _binding.apply {
                btnContinue.isEnabled = false
                btnContinue.isClickable = false
                btnLogout.isEnabled = false
                btnLogout.isClickable = false

                tvSerialOldBattery.text = oldBatSerial
                tvSerialNewBattery.text = newBatSerial
                tvPercentOldBattery.text = if (oldBatSOC != 0) {
                    "${oldBatSOC?.toString() ?: "-"}%"
                } else {
                    "${getSocBatteryFromJNI(getIndexFromBE(oldCab))}%"
                }
                tvPercentNewBattery.text = "${newBatSOC?.toString() ?: "-"}%"
            }
        }
    }
}