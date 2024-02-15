package com.selex.bssStation.model.login

data class LoginResponseModel(
    val id_token: String?,
    val type: String?,
    val title: String?,
    val status: String?,
    val detail: String?,
    val path: String?,
    val message: String?,
    )