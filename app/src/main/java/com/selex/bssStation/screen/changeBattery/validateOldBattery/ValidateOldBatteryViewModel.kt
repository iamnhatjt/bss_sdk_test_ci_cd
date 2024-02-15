package com.selex.bssStation.screen.changeBattery.validateOldBattery

import android.os.CountDownTimer
import android.util.Log
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.selex.bssStation.core.repository.ChangePinRepository
import com.selex.bssStation.core.utils.Constants
import com.selex.bssStation.core.utils.TAG
import com.selex.bssStation.core.utils.ERROR_BATTERY
import com.selex.bssStation.core.utils.NetworkResult
import com.selex.bssStation.model.confirmChangeBattery.ConfirmChangeBatteryModel
import com.selex.bssStation.model.error.ErrorResponse
import com.selex.bssStation.model.newBatteryCabinet.NewBatteryCabinetRequestModel
import com.selex.bssStation.service.*
import dagger.hilt.android.lifecycle.HiltViewModel
import getIndexFromBE
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.greenrobot.eventbus.EventBus
import org.greenrobot.eventbus.Subscribe
import org.greenrobot.eventbus.ThreadMode
import org.json.JSONObject
import javax.inject.Inject

@HiltViewModel
class ValidateOldBatteryViewModel @Inject constructor(
    private val changePinRepository: ChangePinRepository
) : ViewModel() {
    private val _responseConfirmChangeBattery: MutableLiveData<NetworkResult<ConfirmChangeBatteryModel>> = MutableLiveData()
    val responseConfirmChangeBattery: LiveData<NetworkResult<ConfirmChangeBatteryModel>> = _responseConfirmChangeBattery
    var transactionId = -1
    var newCabinetId = -1
    private var countDownTimer: CountDownTimer? = null
    val canNotReadBatteryTrigger = MutableLiveData<ERROR_BATTERY>()
    val errorTrigger = MutableLiveData<ErrorResponse?>()
    private var  vehicleSerial = ""
    private var count = 0
    private var isCallConfirm = false
    private fun confirmChangeBattery() = viewModelScope.launch {
         Log.d(TAG, "ValidateOldBatteryViewModel: confirmChangeBattery")
        if (!isCallConfirm) {
            isCallConfirm = true
            changePinRepository.confirmChangeBattery(transactionId.toString()).collect { values ->
                _responseConfirmChangeBattery.value = values
            }
        }
    }

    fun getSerial(availableCabinetId: Int)  {
        handleGetSerialBattery(availableCabinetId)
    }

    private fun handleGetSerialBattery(availableCabinetId: Int) {
        countDownTimer = object : CountDownTimer(15000L, 500) {
            override fun onTick(millisUntilFinished: Long) {
                val timer = millisUntilFinished/500
                val serial = getSerialBattery(availableCabinetId) ?: ""
                if (serial.isNotEmpty()) {
                    Log.d(TAG, "ValidateOldBatteryViewModel: SERIAL CALL API: $serial")
                    var obj = JSONObject(BackgroundService.resultBatteryConnected)
                    var id = obj["id"]
                    if (id.toString() == availableCabinetId.toString()) {
                        Log.d(TAG, "ValidateOldBatteryViewModel: SERIAL CALL API: call api $serial")
                        countDownTimer?.cancel()
                        getNewBatteryCabinet(serial, availableCabinetId)
                    }

                }  else if (timer.toInt() == 20 || timer.toInt() == 10) {
                    Log.d(TAG, "ValidateOldBatteryViewModel: Try read BP")
                    tryReadBp(availableCabinetId, count = 1)
                }
            }
            override fun onFinish() {
                Log.d(TAG, "ValidateOldBatteryViewModel: handleGetSerialBattery() onFinish")
                canNotReadBatteryTrigger.value = ERROR_BATTERY.CANNOTREAD
            }
        }.start()
    }

    init {
        EventBus.getDefault().register(this)
    }

    override fun onCleared() {
        super.onCleared()
        EventBus.getDefault().unregister(this)
    }

    @Subscribe(threadMode = ThreadMode.BACKGROUND)
    fun onMessageEvent(event: MessageEvent) {
        if (event.message == Constants.ASSIGN_BATTERY_SUCCESS) {
            Log.d(TAG, "ValidateOldBatteryViewModel: Subscribe event ASSIGN_BATTERY_SUCCESS")
            try {
                val obj = JSONObject(event.data.toString())
                val id = obj["id"]
                val sn = obj["sn"]
                val isSuccess = obj["isSuccess"]

                if (id.toString() == newCabinetId.toString()) {
                    if (isSuccess == 0) {
                        if (count < 3) {
                            count ++
                            Log.d(TAG, "ValidateOldBatteryViewModel write failed $obj")
                            cabAssignedDevicesToBpFromJNI(newCabinetId, vehicleSerial)
                        } else {
                            Log.d(TAG, "ValidateOldBatteryViewModel write failed $obj by pass")
                            count = 0
                            confirmChangeBattery()
                        }

                    } else {
                        Log.d(TAG, "ValidateOldBatteryViewModel: cabAssignedDevicesToBpFromJNI $obj")
                        count = 0
                        confirmChangeBattery()
                    }
                }
            } catch (e: Exception) {
                Log.d(TAG, "ValidateOldBatteryViewModel: error parse json")
                e.printStackTrace()
            }
        }
    }

    fun getNewBatteryCabinet(batterySerial: String, availableCabinetId: Int) = viewModelScope.launch {
        val serial = getSerialNumberBssFromJNI()
        val soc = getSocBatteryFromJNI(availableCabinetId)
        val request = NewBatteryCabinetRequestModel(serial, batterySerial, soc)
        Log.d(TAG, "ValidateOldBatteryViewModel:getNewBatteryCabinet, $batterySerial $soc")
        changePinRepository.getNewBatteryCabinet(request, transactionId.toString()).collect { values ->
            if (values.data != null) {
                if (values.data.vehicleSerial?.isNotEmpty() == true) {
                    vehicleSerial = values.data.vehicleSerial
                    viewModelScope.launch {
                         withContext(Dispatchers.IO) {
                             values.data.availableCabinetId?.let {
                                 newCabinetId = getIndexFromBE(it)
                                 cabAssignedDevicesToBpFromJNI(
                                     getIndexFromBE(it),
                                     values.data.vehicleSerial)
                             }
                        }

                    }

                } else {
                    errorTrigger.value = ErrorResponse(
                        message = "Serial Vehicle failed",
                        errorKey = "WRONGVEHICLE",
                        status = -1
                    )
                }
            } else {
                errorTrigger.value = values.error
            }
        }
    }
}