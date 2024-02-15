package com.selex.bssStation.core.repository

import com.google.gson.Gson
import com.selex.bssStation.App
import com.selex.bssStation.core.utils.NetWorkUtil
import com.selex.bssStation.core.utils.NetworkResult
import com.selex.bssStation.model.error.ErrorResponse
import retrofit2.Response

abstract class BaseApiResponse {

    suspend fun <T,R> safeApiCall(apiCall: suspend () -> Response<R>,transform :(R) -> T ): NetworkResult<T> {
        try {
            val response = apiCall()
            if (response.isSuccessful) {
                val body = response.body()
                body?.let {
                    return NetworkResult.Success(transform(body))
                }
            }
            return error("${response.code()} ${response.message()}")
        } catch (e: Exception) {
            return error(e.message ?: e.toString())
        }
    }

    suspend fun <T> safeApiCall(apiCall: suspend () -> Response<T>): NetworkResult<T> {
        if(NetWorkUtil.isNetworkAvailable(App.context)){
            try {
                val response = apiCall()
                if (response.isSuccessful) {
                    val body = response.body()
                    body?.let {
                        return NetworkResult.Success(body)
                    }
                }

                val error: ErrorResponse? = try {
                    Gson().fromJson(response.errorBody()?.string(), ErrorResponse::class.java)
                } catch (e: Exception) {
                    null
                }
                return error(error = error)
            } catch (e: Exception) {
                val err = ErrorResponse(errorKey = "NO_INTERNET", message = "Mất kết nối mạng, vui lòng thử lại sau!", status = -1);
                return  NetworkResult.Error(err)
            }
        } else {
            val err = ErrorResponse(errorKey = "NO_INTERNET", message = "Mất kết nối mạng, vui lòng thử lại sau!", status = -1);
            return  NetworkResult.Error(err)
        }
    }

    private fun <T> error(error: ErrorResponse?): NetworkResult<T> =
        NetworkResult.Error(error)


}