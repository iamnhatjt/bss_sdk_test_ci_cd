package com.selex.bssStation.core.repository

import com.selex.bssStation.core.service.BssService
import com.selex.bssStation.core.utils.NetworkResult
import com.selex.bssStation.model.banner.BannerModel
import com.selex.bssStation.model.bss.BssStatusModel
import com.selex.bssStation.model.bssInfo.BssInfo
import com.selex.bssStation.model.login.LoginQrRequestModel
import javax.inject.Inject
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.flow.flowOn


class BssRepository @Inject constructor(private val bssService: BssService) :
    BaseApiResponse() {

    suspend fun getBannerStation(): Flow<NetworkResult<List<BannerModel>>> {
        return flow {
            emit(safeApiCall { bssService.getBannerStation() })
        }.flowOn(Dispatchers.IO)
    }

    suspend fun getBssStatus(serial: String): Flow<NetworkResult<BssStatusModel>> {
        return flow {
            emit(safeApiCall { bssService.getBssStatus(serial) })
        }.flowOn(Dispatchers.IO)
    }

    suspend fun getNumberPinAvailable(serial: String): Flow<NetworkResult<BssInfo>> {
        return flow {
            emit(safeApiCall { bssService.getNumberPinAvailable(serial) })
        }.flowOn(Dispatchers.IO)
    }
}
