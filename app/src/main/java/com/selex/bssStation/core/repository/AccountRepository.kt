package com.selex.bssStation.core.repository

import com.selex.bssStation.core.service.AccountService
import com.selex.bssStation.core.utils.NetworkResult
import com.selex.bssStation.model.login.*
import com.selex.bssStation.model.user.UserModel
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.flow.flowOn
import javax.inject.Inject
import kotlin.time.Duration.Companion.milliseconds
import kotlin.time.Duration.Companion.seconds

class AccountRepository @Inject constructor(private val accountService: AccountService) :
    BaseApiResponse() {

    suspend fun getUserInfo(serial: String): Flow<NetworkResult<UserModel>> {
        return flow {
            emit(safeApiCall { accountService.getUserInfo(serial) })
        }.flowOn(Dispatchers.IO)
    }

    suspend fun login(user: LoginRequestModel): Flow<NetworkResult<LoginResponseModel>> {
        return flow {
            emit(NetworkResult.Loading())
            emit(safeApiCall { accountService.loginAccount(user) })
        }.flowOn(Dispatchers.IO)
    }

    suspend fun generateQr(request: LoginQrRequestModel): Flow<NetworkResult<LoginQRModel>> {
        return flow {
            emit(NetworkResult.Loading())
            delay(300.milliseconds)
            emit(safeApiCall { accountService.generateQr(request) })
        }.flowOn(Dispatchers.IO)
    }

    suspend fun pollingQR(id: Int): Flow<NetworkResult<LoginQRPollingModel>> {
        return flow {
            emit(safeApiCall { accountService.pollingQR(id) })
        }.flowOn(Dispatchers.IO)
    }
}