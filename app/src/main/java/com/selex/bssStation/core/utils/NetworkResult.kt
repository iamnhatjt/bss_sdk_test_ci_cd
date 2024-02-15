package com.selex.bssStation.core.utils

import com.selex.bssStation.model.error.ErrorResponse

sealed class NetworkResult<T>(
    val data: T? = null,
    val error: ErrorResponse? = null,
) {
    class Success<T>(data: T) : NetworkResult<T>(data)
    class Error<T>(error: ErrorResponse?,  data: T? = null, errorKey: String? = null) : NetworkResult<T>(data, error)
    class Loading<T> : NetworkResult<T>()
}