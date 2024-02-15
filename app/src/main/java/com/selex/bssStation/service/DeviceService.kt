package com.selex.bssStation.service

import android.util.Log
import com.selex.bssStation.core.utils.TAG
import com.selex.bssStation.core.utils.properties.SystemProperties

//Get Properties from System
fun getSerialNumberBssFromJNI(): String {
    return SystemProperties.read(SystemProperties.SERIAL_NUMBER)
}
fun getFactoryDateFromJNI(): String {
    return SystemProperties.read(SystemProperties.FACTORY_DATE)
}
fun getHwVersionFromJNI(): String {
    return SystemProperties.read(SystemProperties.HW_VERSION)
}
external fun getLotNumberFromJNI(): String?
fun getModelFromJNI(): String {
    return SystemProperties.read(SystemProperties.MODEL)
}
//get configuration

fun setDoorOpen(id: Int): Int {
    Log.d(TAG, "setDoorOpen from APP")
    return setDoorOpenFromJNI(id)
}
external fun setDoorOpenFromJNI(id: Int): Int

external fun tryReadBp(id: Int, count : Int = 1): Int
external fun getStateBssFromJNI(): Int

external fun emeterGetVol(): Float
external fun emeterGetCur(): Float
external fun emeterGetCos(): Float
external fun emeterGetFreq(): Float
external fun emeterGetEnergy(): Float

external fun chargerGetNumberBssJNI(): Int
external fun chargerGetVol(id: Int) : Int
external fun chargerGetCur(id: Int) : Int
external fun chargerGetChargingCab(id: Int) : Int
external fun chargerGetState(id: Int) : Int
external fun chargerCharge( cabId: Int) : Int
external fun chargerDischarge( cabId: Int) : Int
external fun chargerCabEnable(id: Int, cabId: Int, boolean: Boolean) : Int
external fun getNumberCabinetFromJNI(): Int
external fun getReadyPackFromJNI(): Int
external fun getTotalPackFromJNI(): Int
external fun getDoorStateFromJNI(id: Int): Int
external fun getFanStateFromJNI(id: Int): Int
external fun getOpStateFromJNI(id: Int): Int

external fun getIsChargedFromJNI(id: Int): Int
external fun getStateCabinetFromJNI(id: Int): Int
external fun getTempCabinetListFromJNI(id: Int): IntArray
external fun getTempCabinetThresholdFromJNI(): Int
external fun getSerialBatteryFromJNI(id: Int): String?

fun getSerialBattery(id: Int): String? {
    val state = getDoorStateFromJNI(id)
    if (state == 0) {
        if (BackgroundService.resultDoorChange != null) {
            val idNow: Int = BackgroundService.resultDoorChange!!["id"] as Int
            val status = BackgroundService.resultDoorChange!!["status"]
            if (idNow == id && status == 1) {
                return ""
            }
        }
        val soc = getSocBatteryFromJNI(id)
        if (soc <= 0) {
            return ""
        } else {
            return getSerialBatteryFromJNI(id)
        }
    } else {
        return ""
    }
}
external fun getSocBatteryFromJNI(id: Int): Int
external fun processBssFromJNY()
external fun cabAssignedDevicesToBpFromJNI(id: Int, serial: String): Int

external fun getInfoBp(id: Int): String?

external fun getTempBp(id: Int): IntArray
external fun getCellVol(id: Int): IntArray

external fun getAssignedSn(id: Int): String?
//external fun getTempsBp(id: Int): IntArray
external fun getListBp(): String?
