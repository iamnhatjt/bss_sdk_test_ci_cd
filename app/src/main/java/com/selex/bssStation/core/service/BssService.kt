package com.selex.bssStation.core.service

import com.selex.bssStation.model.banner.BannerModel
import com.selex.bssStation.model.bss.BssStatusModel
import com.selex.bssStation.model.bssInfo.BssInfo
import retrofit2.Response
import retrofit2.http.GET
import retrofit2.http.Path

interface BssService {
    @GET("services/contentservice/api/public/banners/BSS")
    suspend fun getBannerStation(): Response<List<BannerModel>>

    @GET("services/bssservice/api/v2/public/bss/{serial}")
    suspend fun getBssStatus(@Path("serial") serial: String): Response<BssStatusModel>

    @GET("services/deviceshadow/api/public/device-shadows/{serial}/get")
    suspend fun getNumberPinAvailable(@Path("serial") serial: String): Response<BssInfo>
}
