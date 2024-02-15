package com.selex.bssStation.screen.home

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.selex.bssStation.core.repository.AccountRepository
import com.selex.bssStation.core.repository.ChangePinRepository
import com.selex.bssStation.core.utils.NetworkResult
import com.selex.bssStation.model.changeBattery.EmptyCabinetModel
import com.selex.bssStation.model.user.UserModel
import com.selex.bssStation.service.getSerialNumberBssFromJNI
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class MainViewModel @Inject constructor(
    private val changePinRepository: ChangePinRepository,
    private val accountRepository: AccountRepository
) : ViewModel() {

    private val _responseEmptyCabinet: MutableLiveData<NetworkResult<EmptyCabinetModel>> = MutableLiveData()
    val responseEmptyCabinet: LiveData<NetworkResult<EmptyCabinetModel>> = _responseEmptyCabinet

    fun getEmptyCabinet() = viewModelScope.launch {
        changePinRepository.getEmptyCabinet(getSerialNumberBssFromJNI()).collect { values ->
            _responseEmptyCabinet.value = values
        }
    }

}