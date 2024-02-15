package com.selex.bssStation.screen.standBy

import android.os.CountDownTimer
import android.util.Log
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.selex.bssStation.core.repository.BssRepository
import com.selex.bssStation.core.utils.Constants
import com.selex.bssStation.core.utils.TAG
import com.selex.bssStation.core.utils.NetworkResult
import com.selex.bssStation.core.utils.SharedPreferences
import com.selex.bssStation.model.banner.BannerModel
import com.selex.bssStation.model.bss.BssStatusModel
import com.selex.bssStation.model.bssInfo.BssInfo
import com.selex.bssStation.screen.Screen
import com.selex.bssStation.screen.ScreenManager
import com.selex.bssStation.service.*
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.launch
import org.greenrobot.eventbus.EventBus
import org.greenrobot.eventbus.Subscribe
import org.greenrobot.eventbus.ThreadMode
import org.json.JSONObject
import javax.inject.Inject

@HiltViewModel
class ScreenStandbyViewModel @Inject constructor(
    private val bssRepository: BssRepository
) : ViewModel() {
    private val _response: MutableLiveData<NetworkResult<List<BannerModel>>> = MutableLiveData()
    val response: LiveData<NetworkResult<List<BannerModel>>> = _response

    private val _bssStatusResponse = MutableLiveData<NetworkResult<BssStatusModel>>()
    val bssStatusResponse: LiveData<NetworkResult<BssStatusModel>> get() = _bssStatusResponse

    private val _numberPinResponse = MutableLiveData<NetworkResult<BssInfo>>()
    val numberPinResponse: LiveData<NetworkResult<BssInfo>> get() = _numberPinResponse

    @Inject
    lateinit var sharedPreferences: SharedPreferences

    private val _numberPin = MutableLiveData<Int>()
    val numberPin: LiveData<Int> get() = _numberPin

    private val _showLayoutMaintain = MutableLiveData<Boolean>()
    val showLayoutMaintain: LiveData<Boolean> get() = _showLayoutMaintain
    private var countDownTimer: CountDownTimer? = null
    val showSnackbar = MutableLiveData<Boolean>()
    init {
        EventBus.getDefault().register(this)
    }

    override fun onCleared() {
        EventBus.getDefault().unregister(this)
        super.onCleared()
    }

    private fun validateNewPin(idCabinet: Int) {
        Log.d(TAG, "${javaClass.simpleName}: Validate new battery")
        countDownTimer?.cancel()
        countDownTimer = object : CountDownTimer(5000L, 500) {
            override fun onTick(millisUntilFinished: Long) {
                val serial = getSerialBattery(idCabinet) ?: ""
                if (serial.isNotEmpty()) {
                    var obj = JSONObject(BackgroundService.resultBatteryConnected)
                    var id = obj["id"]
                    Log.d(TAG, "${javaClass.simpleName}: Return battery $id  $idCabinet")
                    if (id.toString() == idCabinet.toString()) {
                        val result = setDoorOpenFromJNI(idCabinet)
                        countDownTimer?.onFinish()
                        Log.d(TAG, "${javaClass.simpleName}: Return battery $result")
                        if (result < 0) {
                            setDoorOpenFromJNI(idCabinet)
                            showWarning()
                        } else {
                            showWarning()
                        }
                    }
                }
            }

            override fun onFinish() {

            }
        }.start()
    }

    private fun showWarning() {
        showSnackbar.value = true
    }

    @Subscribe(threadMode = ThreadMode.BACKGROUND)
    fun onMessageEvent(event: MessageEvent) {
        when {
            event.message == Constants.BSS_INFO_CHANGE -> {
                getBssStatus()
            }
            event.message == Constants.BSS_BANNER_CHANGE -> {
                getBanner()
            }
            event.message.contains("NUMBER_PIN") -> {
                val pin = event.message.split(":").last()
            //    Log.d(LOG_TAG, "NUMBER PIN $pin")
                viewModelScope.launch {
                    _numberPin.value = pin.toInt()
                }
            }
            event.message == Constants.BATTERY_CONNECTED -> {
            }
            event.message == Constants.BSS_DOOR_CHANGE -> {
//                if (sharedPreferences.getToken().trim() == "" && ScreenManager.currentScreen == Screen.STAND_BY) {
//                    try {
//                        val obj = JSONObject(event.data.toString())
//                        val id: Int = obj["id"] as Int
//                        val status = obj["status"]
//                        if (status == 0) {
//                            viewModelScope.launch {
//                                validateNewPin(id)
//                            }
//                        }
//                    } catch (e: Exception) {
//                        e.printStackTrace()
//                    }
//                }
            }
            event.message == Constants.BATTERY_STATION_MAINTAINING -> {
                viewModelScope.launch {
                    _showLayoutMaintain.value = true
                }
            }
        }
    }

    fun getBanner() = viewModelScope.launch {
        bssRepository.getBannerStation().collect { values ->
            _response.value = values
        }
    }

    fun getBssStatus() = viewModelScope.launch {
        val serial = getSerialNumberBssFromJNI()
        if (serial.isNotEmpty()) {
            bssRepository.getBssStatus(serial).collect { values ->
                _bssStatusResponse.value = values
            }
        }
    }

    fun getNumberPinAvailable() = viewModelScope.launch {
        val serial = getSerialNumberBssFromJNI()
        if (serial.isNotEmpty()) {
            bssRepository.getNumberPinAvailable(serial).collect { values ->
                if (values.data != null) {
                    _numberPinResponse.value = values
                }
            }
        }
    }
}