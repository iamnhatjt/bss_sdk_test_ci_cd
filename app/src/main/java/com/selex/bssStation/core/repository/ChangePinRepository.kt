package com.selex.bssStation.core.repository

import com.selex.bssStation.core.service.ChangeBatteryService
import com.selex.bssStation.core.utils.NetworkResult
import com.selex.bssStation.model.changeBattery.EmptyCabinetModel
import com.selex.bssStation.model.confirmChangeBattery.ConfirmChangeBatteryModel
import com.selex.bssStation.model.newBatteryCabinet.NewBatteryCabinetModel
import com.selex.bssStation.model.newBatteryCabinet.NewBatteryCabinetRequestModel
import com.selex.bssStation.service.BackgroundService
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.flow.flowOn
import javax.inject.Inject

class ChangePinRepository @Inject constructor(private val changeBatteryService: ChangeBatteryService) :
    BaseApiResponse() {
    suspend fun getEmptyCabinet(serial: String, cabinetId: Int? = null): Flow<NetworkResult<EmptyCabinetModel>> {
        return flow {
            emit(safeApiCall { changeBatteryService.getEmptyCabinet(serial, cabinetId, BackgroundService.bssToBe()) })
        }.flowOn(Dispatchers.IO)
    }

    suspend fun getNewBatteryCabinet(request: NewBatteryCabinetRequestModel, transactionId: String): Flow<NetworkResult<NewBatteryCabinetModel>> {
        return flow {
            emit(safeApiCall { changeBatteryService.getNewBatteryCabinet(transactionId, request = request) })
        }.flowOn(Dispatchers.IO)
    }
    suspend fun confirmChangeBattery(transactionId: String): Flow<NetworkResult<ConfirmChangeBatteryModel>> {
        return flow {
            emit(safeApiCall { changeBatteryService.confirmChangeBattery(transactionId) })
        }.flowOn(Dispatchers.IO)
    }
}
