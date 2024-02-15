package com.selex.bssStation.screen.changeBattery.validateOldBattery

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.os.CountDownTimer
import android.os.Handler
import android.util.Log
import androidx.activity.viewModels
import androidx.databinding.DataBindingUtil
import com.selex.bssStation.R
import com.selex.bssStation.databinding.ActivityLoadingBinding
import com.selex.bssStation.screen.base.BaseActivity
import com.selex.bssStation.screen.changeBattery.resultSuccess.ResultSucessActivity
import com.selex.bssStation.screen.login.loginAccount.LoginAccountActivity
import com.selex.bssStation.service.cabAssignedDevicesToBpFromJNI
import com.selex.bssStation.service.getSerialBattery
import android.view.animation.AnimationUtils
import android.view.animation.LinearInterpolator
import androidx.lifecycle.MutableLiveData
import com.selex.bssStation.core.utils.Constants
import com.selex.bssStation.core.utils.TAG
import com.selex.bssStation.core.utils.ERROR_BATTERY
import com.selex.bssStation.model.confirmChangeBattery.ConfirmChangeBatteryModel
import com.selex.bssStation.model.error.ErrorResponse
import com.selex.bssStation.screen.Screen
import com.selex.bssStation.screen.ScreenManager
import com.selex.bssStation.screen.changeBattery.errorBattery.BatteryErrorActivity
import com.selex.bssStation.screen.changeBattery.openNewPin.OpenNewPinActivity
import com.selex.bssStation.screen.home.MainActivity
import com.selex.bssStation.service.getSocBatteryFromJNI
import getIndexFromBE
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import setIndexCabinetToBE

class ValidateOldBatteryActivity : BaseActivity() {

    private lateinit var _binding: ActivityLoadingBinding
    private var oldCabinetId = -1
    private var transactionId = -1
    private val viewModel: ValidateOldBatteryViewModel by viewModels()
    private var countDownTimeOut: CountDownTimer? = null

    companion object {
        fun startActivity(activity: Activity, bundle: Bundle? = null, oldCabinetId: Int, transactionId: Int) {
            val intent = Intent(activity, ValidateOldBatteryActivity::class.java)
            if (bundle!= null) {
                intent.putExtras(bundle)
            }
            intent.putExtra("oldCabinetId", oldCabinetId);
            intent.putExtra("transactionId", transactionId);
            intent.flags = Intent.FLAG_ACTIVITY_SINGLE_TOP or Intent.FLAG_ACTIVITY_CLEAR_TOP
            activity.startActivity(intent)
        }
    }

    @SuppressLint("SuspiciousIndentation")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.i(TAG, "ValidateOldBatteryActivity: onCreate")
        _binding = DataBindingUtil.setContentView(this, R.layout.activity_loading)
        oldCabinetId = intent.getIntExtra("oldCabinetId", -1)
        viewModel.transactionId =  intent.getIntExtra("transactionId", -1)
        transactionId = intent.getIntExtra("transactionId", -1)
        timeOut()
        viewModel.getSerial(oldCabinetId)

        viewModel.canNotReadBatteryTrigger.observe(this) {
            if (it == ERROR_BATTERY.CANNOTREAD) {
                BatteryErrorActivity.startActivity(this,  oldCabinetId = oldCabinetId, transactionId = transactionId)
                finish()
            }
        }
        viewModel.errorTrigger.observe(this) {
            if (it != null) {
                BatteryErrorActivity.startActivity(this,  oldCabinetId = oldCabinetId, transactionId = transactionId, error = it)
                finish()
            }
        }

        viewModel.responseConfirmChangeBattery.observe(this) {
            if (it.data != null) {
                goToNewPin(it.data)
            } else {
                BatteryErrorActivity.startActivity(this,  oldCabinetId = oldCabinetId, transactionId = transactionId, error = it.error)
                finish()
            }
        }
    }

    private fun goToNewPin(model: ConfirmChangeBatteryModel) {
        OpenNewPinActivity.startActivity(this, model = model)
        finish()
    }
    override fun onResume() {
        super.onResume()
        Log.i(TAG, "ValidateOldBatteryActivity: OnResume")
        handleRotateAnimation()
        ScreenManager.currentScreen = Screen.VALIDATE_OLD_BATTERY
    }

    override fun onPause() {
        Log.i(TAG, "ValidateOldBatteryActivity: OnPause")
        super.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.i(TAG, "ValidateOldBatteryActivity: OnDestroy")
    }


    override fun finish() {
        super.finish()
        Log.i(TAG, "ValidateOldBatteryActivity: finish")
        countDownTimeOut?.cancel()
        overridePendingTransition(R.anim.left_in, R.anim.right_out)
    }
    private fun handleRotateAnimation() {
        val animationRotate = AnimationUtils.loadAnimation(this, R.anim.animation_rotate)
        animationRotate.startOffset = 0
        animationRotate.duration = 1500
        _binding.layoutLoadingCheckBattery.imgLogoSelexWhite.startAnimation(animationRotate)
    }

    private fun timeOut() {
        Log.d(TAG, "ValidateOldBattery start timeout")
        countDownTimeOut = object : CountDownTimer(60000L, 1000) {
            @SuppressLint("SetTextI18n")
            override fun onTick(millisUntilFinished: Long) {

            }
            override fun onFinish() {
                Log.d(TAG, "ValidateOldBattery start timeout onFinish")
                finish()
                BatteryErrorActivity.startActivity(this@ValidateOldBatteryActivity,  oldCabinetId = oldCabinetId, transactionId = transactionId, error = ErrorResponse("Có lỗi xảy ra, vui lòng thử lại", errorKey = "WRONGID", status = -1 ))
            }
        }.start()
    }

}