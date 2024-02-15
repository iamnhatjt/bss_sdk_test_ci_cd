package com.selex.bssStation.screen.changeBattery.resultSuccess

import android.util.Log
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.selex.bssStation.core.repository.ChangePinRepository
import com.selex.bssStation.core.utils.NetworkResult
import com.selex.bssStation.core.utils.TAG
import com.selex.bssStation.model.changeBattery.EmptyCabinetModel
import com.selex.bssStation.service.getSerialNumberBssFromJNI
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class ResultSucessViewModel @Inject constructor(
    private val changePinRepository: ChangePinRepository,
) : ViewModel() {

    private val _responseEmptyCabinet: MutableLiveData<NetworkResult<EmptyCabinetModel>> = MutableLiveData()
    val responseEmptyCabinet: LiveData<NetworkResult<EmptyCabinetModel>> = _responseEmptyCabinet
    private var isCallAPI = false

    fun createNewTransactionId(oldCabinetId: Int) = viewModelScope.launch {
        if (!isCallAPI) {
            isCallAPI = true
            Log.d(TAG, "createNewTransactionId")
            changePinRepository.getEmptyCabinet(getSerialNumberBssFromJNI(), cabinetId = oldCabinetId).collect { values ->
                if (_responseEmptyCabinet.value?.data == null) {
                    _responseEmptyCabinet.value = values
                    isCallAPI = false
                }
            }
        }
    }

}