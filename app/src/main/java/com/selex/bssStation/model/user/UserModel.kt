package com.selex.bssStation.model.user

import com.selex.bssStation.core.utils.Constants
import com.selex.bssStation.core.utils.TAG
import java.io.Serializable

data class UserModel(
    val account: AccountModel?,
    val subscription: SubscriptionModel?
): Serializable

fun UserModel?.isGuarantor(): Boolean {
    return this?.account?.authorities?.contains(Constants.ROLE_GUARANTOR) == true
}

val defaultGuarantorModel = UserModel(
    account = AccountModel(
        activated = null,
        authorities = listOf(Constants.ROLE_GUARANTOR),
        address = null,
        createdBy = null,
        createdDate = null,
        dob = null,
        email = null,
        firstName = "",
        lastName = "Bảo hành",
        fullname = null,
        gender = null,
        id = null,
        identityNumber = null,
        imageUrl = null,
        langKey = null,
        lastModifiedBy = null,
        lastModifiedDate = null,
        login = null,
        phone = null
    ),
    subscription = null
)

data class AccountModel(
    val activated: Boolean?,
    val address: String?,
    val authorities: List<String>?,
    val createdBy: String?,
    val createdDate: String?,
    val dob: String?,
    val email: String?,
    val firstName: String?,
    val fullname: String?,
    val gender: String?,
    val id: Int?,
    val identityNumber: String?,
    val imageUrl: String?,
    val langKey: String?,
    val lastModifiedBy: String?,
    val lastModifiedDate: String?,
    val lastName: String?,
    val login: String?,
    val phone: String?
): Serializable
data class SubscriptionModel(
    val status: String?,
): Serializable