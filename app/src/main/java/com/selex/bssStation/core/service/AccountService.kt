package com.selex.bssStation.core.service

import com.selex.bssStation.model.login.*
import com.selex.bssStation.model.user.UserModel
import retrofit2.Response
import retrofit2.http.Body
import retrofit2.http.GET
import retrofit2.http.POST
import retrofit2.http.Path

interface AccountService {
    @POST("api/session-infos/qr/generate")
    suspend fun generateQr(@Body request: LoginQrRequestModel): Response<LoginQRModel>

    @POST("api/authenticate")
    suspend fun loginAccount(@Body loginRequest: LoginRequestModel): Response<LoginResponseModel>

    @GET("services/bssservice/api/v2/bsses/{serial}/info")
    suspend fun getUserInfo(@Path("serial") serial: String): Response<UserModel>

    @GET("api/session-infos/qr/polling/{id}")
    suspend fun pollingQR(@Path("id") id: Int): Response<LoginQRPollingModel>

}
