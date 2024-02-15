package com.selex.bssStation.screen.changeBattery.openNewPin

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.os.CountDownTimer
import android.os.Handler
import android.os.Looper
import android.util.Log
import androidx.activity.viewModels
import androidx.databinding.DataBindingUtil
import androidx.recyclerview.widget.GridLayoutManager
import com.selex.bssStation.R
import com.selex.bssStation.core.utility.BottomDialogFragment
import com.selex.bssStation.core.utils.Constants
import com.selex.bssStation.core.utils.TAG
import com.selex.bssStation.databinding.ActivityResultNewbatteryBinding
import com.selex.bssStation.model.confirmChangeBattery.ConfirmChangeBatteryModel
import com.selex.bssStation.screen.Screen
import com.selex.bssStation.screen.ScreenManager
import com.selex.bssStation.screen.base.BaseActivity
import com.selex.bssStation.screen.changeBattery.emptyCompartment.ListCompartmentAdapter
import com.selex.bssStation.screen.changeBattery.resultSuccess.ResultSucessActivity
import com.selex.bssStation.screen.changeBattery.validateOldBattery.ValidateOldBatteryActivity
import com.selex.bssStation.screen.home.MainActivity
import com.selex.bssStation.service.*
import dagger.hilt.android.AndroidEntryPoint
import getIndexFromBE
import kotlinx.coroutines.*
import org.greenrobot.eventbus.Subscribe
import org.greenrobot.eventbus.ThreadMode
import org.json.JSONObject
import setIndexCabinetToBE


@AndroidEntryPoint
class OpenNewPinActivity : BaseActivity() {
    private lateinit var _binding: ActivityResultNewbatteryBinding
    private var listCompartmentAdapter: ListCompartmentAdapter? = null
    private var newCabinetId = -1
    private var model: ConfirmChangeBatteryModel? = null
   // private val viewModel: OpenNewPinViewModel by viewModels()
    private var countDownTimerOpenDoor: CountDownTimer? = null
    private var countDownTimer: CountDownTimer? = null
//    private var countDownTimerAgain: CountDownTimer? = null
    private var goToResult = false
  //  private var goToConfirm = false
    private var count = 0
    private var serialOld = ""
    companion object {
        fun startActivity(
            activity: Activity,
            bundle: Bundle? = null,
            model: ConfirmChangeBatteryModel,
        ) {
            val intent = Intent(activity, OpenNewPinActivity::class.java)
            if (bundle != null) {
                intent.putExtras(bundle)
            }
            intent.putExtra("CONFIRM_PIN_RESULT", model)
            activity.startActivity(intent)
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.i(TAG, "OpenNewPinActivity: OnCreate")

        _binding = DataBindingUtil.setContentView(this, R.layout.activity_result_newbattery)

        if (intent.hasExtra("CONFIRM_PIN_RESULT")) {
            model = intent.getSerializableExtra("CONFIRM_PIN_RESULT") as? ConfirmChangeBatteryModel
        }
        model?.newCab?.let { newCabinetId = getIndexFromBE(it) }
        serialOld = model?.newBatSerial.toString()
        _binding.tvPositionOpen.text = "Đã mở cửa khoang số ${setIndexCabinetToBE(newCabinetId)}"
        showListBatteryCompartment()
        GlobalScope.launch {
            handleOpenCabinet()
            assignBatteryOldToBss()
        }
       // Handler().postDelayed({
            handleCheckOpenDoorFirst()
        //}, 3000)
//        viewModel.responseEmptyCabinet.observe(this) {
//
//            if (it.data != null) {
//                it.data.availableCabinetId?.let { it1 ->
//                    it.data.transactionId?.let { it2 ->
//                        ValidateOldBatteryActivity.startActivity(
//                            this,
//                            oldCabinetId = getIndexFromBE(it1),
//                            transactionId = it2
//                        )
//                    }
//                }
//                finish()
//            } else {
//                it.error?.message?.let { it1 ->
//                    _binding.bottomDialogFragment.getFragment<BottomDialogFragment>()
//                        .show(message = it1, title = "Có lỗi xảy ra")
//                }
//                model?.newCab?.let {
//                    setDoorOpenFromJNI(getIndexFromBE(it))
//                }
//                Handler().postDelayed({
//                    Log.d(TAG, "${javaClass.simpleName}: Go to result when Call API failed")
//                    goToResult()
//                }, 3000)
//            }
//        }
    }

    private suspend fun assignBatteryOldToBss() {
        withContext(Dispatchers.IO) {
            try {
                val serial = getSerialNumberBssFromJNI()
                model!!.oldCab?.let { getIndexFromBE(it) }?.let {
                    Log.d(TAG, "OpenNewPinActivity: Assign serial to battery: $it  ${model?.oldCab}")
                    cabAssignedDevicesToBpFromJNI(
                        it,
                        serial!!
                    )
                }
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }

    override fun onResume() {
        super.onResume()
        Log.i(TAG, "OpenNewPinActivity: OnResume")
        ScreenManager.currentScreen = Screen.OPEN_NEW_PIN
    }

    override fun onPause() {
        Log.i(TAG, "OpenNewPinActivity: OnPause")
        super.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.i(TAG, "OpenNewPinActivity: OnDestroy")
    }


    override fun finish() {
        super.finish()
        Log.i(TAG, "OpenNewPinActivity: finish")
        listCompartmentAdapter?.cancelTimerAnimation()
        overridePendingTransition(R.anim.left_in, R.anim.right_out)
    }

    private fun showListBatteryCompartment() {
        _binding.rcyBatteryCompartment.apply {
            layoutManager = GridLayoutManager(context, 4, GridLayoutManager.VERTICAL, false)
            listCompartmentAdapter = ListCompartmentAdapter(animatedIndex = setIndexCabinetToBE(newCabinetId))
            adapter = listCompartmentAdapter
        }
    }

    private suspend fun handleOpenCabinet() {
        withContext(Dispatchers.IO) {
           val result = setDoorOpen(newCabinetId)
            Log.d(TAG, "OpenNewPinActivity: OPEN cabinet $result")
            if (result < 0) {
                Log.d(TAG, "OpenNewPinActivity: OPEN cabinet failed")
               // setDoorOpenFromJNI(newCabinetId)
            }
            serialOld = ""
        }
    }

    private fun handleCheckOpenDoorFirst() {
        Log.d(TAG, "OpenNewPinActivity: Handle check open door first $count")
        countDownTimerOpenDoor = object : CountDownTimer(3000L, 500) {
            override fun onTick(millisUntilFinished: Long) {
            }

            override fun onFinish() {
                val state = getDoorStateFromJNI(newCabinetId)
                if (state == 0) {
                    if (count < 3) {
                        count++
                        handleCheckOpenDoorFirst()
                    } else {
                        Log.d(TAG, "OpenNewPinActivity: Handle check open door first $count open door failed")
                        countDownNavigateScreen()
                    }
                } else {
                    countDownNavigateScreen()
                }
            }
        }.start()
    }

//    //0: dong cua, 1 mo cua
    private fun countDownNavigateScreen() {
        Log.d(TAG, "OpenNewPinActivity: Count down navigate screen")
        countDownTimer = object : CountDownTimer(5000L, 500) {
            override fun onTick(millisUntilFinished: Long) {
            }

            override fun onFinish() {
               goToResult()
            }
        }.start()
    }
//
//    private fun validateAgain() {
//        Log.d(TAG, "${javaClass.simpleName}: Validate again")
//        countDownTimerAgain = object : CountDownTimer(3000L, 500) {
//            override fun onTick(millisUntilFinished: Long) {
//                var serial: String? = getSerialBattery(newCabinetId)
//                val status = getDoorStateFromJNI(newCabinetId)
//                Log.d(TAG, "${javaClass.simpleName}: OPENNEWPIN Validate again: onFinish $serial $serialOld $isBatteryConnected")
//                if (serial?.isNotEmpty() == true && status == 0 && serial != serialOld && isBatteryConnected ) {
//                    countDownTimerAgain?.cancel()
//                    Log.d(TAG, "${javaClass.simpleName}: Validate again: onFinish $serial")
//                        var serial1: String? = getSerialBattery(newCabinetId)
//                        var state1 = getDoorStateFromJNI(newCabinetId)
//                        if (state1 == 0 && serial1?.isNotEmpty() == true && isBatteryConnected) {
//                            changeNextBattery()
//                        } else {
//                            goToResult()
//                        }
//                }
//            }
//
//            override fun onFinish() {
//                Log.d(TAG, "${javaClass.simpleName}: Validate again onFinish, go to result")
//                goToResult()
//            }
//        }.start()
//    }

    private fun goToResult() {
        if (!goToResult) {
            goToResult = true
            finish()
            Log.d(TAG, "OpenNewPinActivity: goToResult")
            ResultSucessActivity.startActivity(this, model = model!!)
        }
    }

//    private fun changeNextBattery() {
//        if (!goToConfirm) {
//            goToConfirm = true
//            model?.newCab?.let {
//                viewModel.createNewTransactionId(it)
//            }
//        }
//    }

}


