package com.selex.bssStation.model.newBatteryCabinet

data class NewBatteryCabinetRequestModel (
    val bssSerial: String?,
    val oldBatSerial: String?,
    val oldBatSoc: Int,
)