package com.selex.bssStation.model.login

data class LoginQRPollingModel(
    val code: Any?,
    val id: Int?,
    val status: String?,
    val token: TokenModel?,
    val user: UserModel?,
    val userId: Int?
)
data class TokenModel(
    val id_token: String?,
    val refresh_token: String?
)

data class UserModel(
    val fullName: String?,
    val imageUrl: Any?
)