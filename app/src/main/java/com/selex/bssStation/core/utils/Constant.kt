package com.selex.bssStation.core.utils

import com.selex.bssStation.BuildConfig

enum class ERROR_BATTERY {
    CANNOTREAD,WRONGIDCABINET, WRITEBPFAILED
}

object Constants {

    const val UPLOAD_FILE_BASE_URL = "http://selex-bss-v2-log.s3.amazonaws.com"
    const val FILE_URL = "http://selex-bss-v2-log.s3-website-ap-southeast-1.amazonaws.com"
    const val BASE_URL = BuildConfig.BASE_URL
    const val HOST_SELEX = BuildConfig.HOST_SELEX
    const val USERNAME_SELEX = "selex"
    const val PASSWORD_SELEX = "selex"

    const val ROLE_GUARANTOR = "ROLE_GUARANTOR"

    const val GUARANTOR_USERNAME_LOCAL = "baohanh"
    const val GUARANTOR_PASSWORD_LOCAL = "Selex@123"

    const val STATUS_NEW = "NEW"
    const val STATUS_SCAN = "SCAN"
    const val STATUS_CONFIRM = "CONFIRM"
    const val STATUS_EXPIRED = "EXPIRED"
    const val STATUS_REJECT = "REJECT"

    const val ACTIVE = "ACTIVE"

        const val BSS_RESTART = "BSS_RESTART"
    const val BSS_RESTART_COMPLETE = "BSS_RESTART_COMPLETE"
    const val BSS_BUSY = "BSS_BUSY"
    const val BSS_INFO_CHANGE = "BSS_INFO_CHANGE"
    const val BSS_BANNER_CHANGE = "BSS_BANNER_CHANGE"
    const val BSS_DOOR_CHANGE = "BSS_DOOR_CHANGE"
    const val BATTERY_CONNECTED = "BATTERY_CONNECTED"
    const val ASSIGN_BATTERY_SUCCESS = "ASSIGN_BATTERY_SUCCESS"
    const val BATTERY_DISCONNECT = "BATTERY_DISCONNECT"
    const val BATTERY_STATION_MAINTAINING = "BATTERY_STATION_MAINTAINING"

    const val COUNTDOWN_TIMER_LOGOUT_WHEN_CHANGE_BATTERY_SUCCESS = 120000L
    const val HAS_NETWORK_KEY = "has_network"
}

const val ACTION_INSTALL_REQUEST = "ACTION_INSTALL_REQUEST"
const val ACTION_REBOOT_REQUEST = "ACTION_REBOOT_REQUEST"

const val TAG = "BSS_STATION"