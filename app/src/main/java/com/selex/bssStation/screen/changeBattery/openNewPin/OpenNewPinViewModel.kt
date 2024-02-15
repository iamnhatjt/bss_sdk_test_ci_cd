package com.selex.bssStation.screen.changeBattery.openNewPin

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.selex.bssStation.core.repository.ChangePinRepository
import com.selex.bssStation.core.utils.NetworkResult
import com.selex.bssStation.model.changeBattery.EmptyCabinetModel
import com.selex.bssStation.service.getSerialNumberBssFromJNI
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class OpenNewPinViewModel @Inject constructor(
    private val changePinRepository: ChangePinRepository,
) : ViewModel() {

//    private val _responseEmptyCabinet: MutableLiveData<NetworkResult<EmptyCabinetModel>> = MutableLiveData()
//    val responseEmptyCabinet: LiveData<NetworkResult<EmptyCabinetModel>> = _responseEmptyCabinet
//
//
//    fun createNewTransactionId(oldCabinetId: Int) = viewModelScope.launch {
//        changePinRepository.getEmptyCabinet(getSerialNumberBssFromJNI(), cabinetId = oldCabinetId).collect { values ->
//            _responseEmptyCabinet.value = values
//        }
//    }
}
