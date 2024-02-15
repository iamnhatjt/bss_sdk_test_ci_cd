package com.selex.bssStation.model.error

import java.io.Serializable

data class ErrorResponse(
    val message: String?,
    val errorKey: String?,
    val status: Int?,
    val code: String? = null
): Serializable