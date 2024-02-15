package com.selex.bssStation.model.confirmChangeBattery

import android.os.Parcel
import android.os.Parcelable
import java.io.Serializable

data class ConfirmChangeBatteryModel(
    val amount: Int?,
    val bssAddress: String?,
    val bssName: String?,
    val bssSerial: String?,
    val bssType: String?,
    val errorLog: String?,
    val id: Int?,
    val newBatSOC: Int?,
    val newBatSerial: String?,
    val oldBatSOC: Int?,
    val oldBatSerial: String?,
    val paymentType: String?,
    val status: String?,
    val user: String?,
    val vehicleColor: String?,
    val vehicleSerial: String?,
    val newCab: Int?,
    val oldCab: Int
): Serializable {
}