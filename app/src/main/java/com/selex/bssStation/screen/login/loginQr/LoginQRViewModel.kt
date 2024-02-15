package com.selex.bssStation.screen.login.loginQr

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.selex.bssStation.core.repository.AccountRepository
import com.selex.bssStation.core.utils.NetworkResult
import com.selex.bssStation.core.utils.SharedPreferences
import com.selex.bssStation.model.login.*
import com.selex.bssStation.model.user.UserModel
import com.selex.bssStation.service.getSerialNumberBssFromJNI
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class LoginQRViewModel @Inject constructor(
    private val accountRepository: AccountRepository
) : ViewModel() {

    private val _qrResponse: MutableLiveData<NetworkResult<LoginQRModel>> = MutableLiveData()
    val qrResponse: LiveData<NetworkResult<LoginQRModel>> = _qrResponse

    private val _responseUser: MutableLiveData<NetworkResult<UserModel>> = MutableLiveData()
    val responseUser: LiveData<NetworkResult<UserModel>> = _responseUser

    private val _pollingQrResponse = MutableLiveData<NetworkResult<LoginQRPollingModel>>()
    val pollingQrResponse : LiveData<NetworkResult<LoginQRPollingModel>> get() = _pollingQrResponse

    @Inject
    lateinit var sharedPreferences: SharedPreferences

    fun generateQr() = viewModelScope.launch {
        val request = LoginQrRequestModel(getSerialNumberBssFromJNI())
        accountRepository.generateQr(request).collect { values ->
            _qrResponse.value = values
        }
    }

    fun getProfile() = viewModelScope.launch {
        accountRepository.getUserInfo(getSerialNumberBssFromJNI()).collect { values ->
            _responseUser.value = values
        }
    }

    fun pollingQr(id: Int) {
        viewModelScope.launch {
            accountRepository.pollingQR(id).collect { values ->

                if (values.data?.token != null) {
                    sharedPreferences.setToken(values.data.token.id_token!!)
                }
                _pollingQrResponse.value = values

            }
        }
    }

}