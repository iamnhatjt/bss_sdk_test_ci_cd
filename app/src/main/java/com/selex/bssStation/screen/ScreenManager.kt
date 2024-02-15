package com.selex.bssStation.screen

object ScreenManager {
    var currentScreen: Screen = Screen.SYNC_DATA
}

enum class Screen {
    SYNC_DATA,
    STAND_BY,
    LOGIN_QR,
    LOGIN_ACCOUNT,
    MAIN,
    VALIDATE_OLD_BATTERY,
    EMPTY_COMPARTMENT,
    OPEN_NEW_PIN,
    RESULT_SUCCESS,
    BATTERY_ERROR
}