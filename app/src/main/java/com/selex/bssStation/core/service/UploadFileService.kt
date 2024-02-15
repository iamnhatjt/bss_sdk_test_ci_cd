package com.selex.bssStation.core.service

import com.selex.bssStation.model.user.UserModel
import okhttp3.MultipartBody
import okhttp3.RequestBody
import retrofit2.Call
import retrofit2.Response
import retrofit2.http.Multipart
import retrofit2.http.POST
import retrofit2.http.Part
import java.util.Objects

interface UploadFileService {

    @Multipart
    @POST("/")
    suspend fun uploadFile(@Part("key") filepathInServer: RequestBody, @Part file: MultipartBody.Part): Response<Unit>

}