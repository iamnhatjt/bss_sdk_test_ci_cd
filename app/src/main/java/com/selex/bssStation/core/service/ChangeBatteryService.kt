package com.selex.bssStation.core.service

import com.selex.bssStation.model.bssInfo.CabinetMqttToBEModel
import com.selex.bssStation.model.bssInfo.ReportedModel
import com.selex.bssStation.model.changeBattery.EmptyCabinetModel
import com.selex.bssStation.model.confirmChangeBattery.ConfirmChangeBatteryModel
import com.selex.bssStation.model.newBatteryCabinet.NewBatteryCabinetModel
import com.selex.bssStation.model.newBatteryCabinet.NewBatteryCabinetRequestModel
import retrofit2.Response
import retrofit2.http.*

interface ChangeBatteryService {
    @POST("services/bssservice/api/v2/bsses/{bssSerial}/tnx")
    suspend fun getEmptyCabinet(@Path("bssSerial") bssSerial: String, @Query("cabinetId") cabinetId: Int?, @Body request: ReportedModel): Response<EmptyCabinetModel>

    @POST("services/bssservice/api/v2/bsses/get-new-cabinet/{transactionId}")
    suspend fun getNewBatteryCabinet(@Path("transactionId") transactionId: String, @Body request: NewBatteryCabinetRequestModel): Response<NewBatteryCabinetModel>
    @PUT("services/swapservice/api/v2/driver/swap-orders/{transactionId}")
    suspend fun confirmChangeBattery(@Path("transactionId") transactionId: String): Response<ConfirmChangeBatteryModel>
}