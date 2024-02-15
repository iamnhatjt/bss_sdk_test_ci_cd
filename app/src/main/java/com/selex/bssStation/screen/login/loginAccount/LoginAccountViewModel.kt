package com.selex.bssStation.screen.login.loginAccount

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope

import com.selex.bssStation.core.repository.AccountRepository
import com.selex.bssStation.core.utils.NetworkResult
import com.selex.bssStation.core.utils.SharedPreferences
import com.selex.bssStation.model.login.LoginRequestModel
import com.selex.bssStation.model.login.LoginResponseModel
import com.selex.bssStation.model.user.UserModel
import com.selex.bssStation.service.getSerialNumberBssFromJNI
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class LoginAccountViewModel @Inject constructor(
    private val accountRepository: AccountRepository
) : ViewModel() {
    private val _response: MutableLiveData<NetworkResult<LoginResponseModel>> = MutableLiveData()
    val response: LiveData<NetworkResult<LoginResponseModel>> = _response

    private val _responseUser: MutableLiveData<NetworkResult<UserModel>> = MutableLiveData()
    val responseUser: LiveData<NetworkResult<UserModel>> = _responseUser

    @Inject
    lateinit var sharedPreferences: SharedPreferences

    fun login(username: String, password: String) = viewModelScope.launch {
        val loginRequest = LoginRequestModel(username, password)
        accountRepository.login(loginRequest).collect { values ->
            if (values.data != null) {
                onLoginResponse(values.data)
            }
            _response.value = values
        }
    }

     fun getProfile() = viewModelScope.launch {
         accountRepository.getUserInfo(getSerialNumberBssFromJNI()).collect { values ->
             _responseUser.value = values
         }
    }

    private fun onLoginResponse(response: LoginResponseModel) {
        sharedPreferences.setToken(response.id_token!!)
    }
}