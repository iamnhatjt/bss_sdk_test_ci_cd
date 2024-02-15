package com.selex.bssStation.core.utils

import android.content.Context
import android.content.SharedPreferences
import androidx.core.content.edit
import javax.inject.Inject

class SharedPreferences @Inject constructor(private val context: Context) {

    private val USER_TOKEN = "user_token"
    private val FIRST_RUN_APP = "FIRST_RUN_APP"
    private val IMAGE_BANNER = "IMAGE_BANNER"
    private val LAST_TIME_RESET_LOGCAT = "LAST_TIME_RESET_LOGCAT"


    private fun getPref(context: Context): SharedPreferences {
        return context.getSharedPreferences(
            context.packageName,
            Context.MODE_PRIVATE
        )
    }

    fun setToken(token: String) {
        val editor: SharedPreferences.Editor = getPref(context)
            .edit()
        editor.putString(USER_TOKEN, token)
        editor.apply()
    }

    fun getToken(): String {
        return getPref(context).getString(
            USER_TOKEN, ""
        )!!
    }

    fun setFirstRun(isFirst: Boolean) {
        val editor: SharedPreferences.Editor = getPref(context).edit()
        editor.putBoolean(FIRST_RUN_APP, isFirst)
        editor.apply()
    }

    fun getFirstRun(): Boolean {
        return getPref(context).getBoolean(
            FIRST_RUN_APP, true
        )
    }

    fun setImageBanner(image: String) {
        val editor: SharedPreferences.Editor = getPref(context).edit()
        editor.putString(IMAGE_BANNER, image)
        editor.apply()
    }

    fun getImageBanner(): String {
        return getPref(context).getString(IMAGE_BANNER, "").toString()
    }

    fun getLastTimeResetLogcat(): Long = getPref(context).getLong(LAST_TIME_RESET_LOGCAT, 0)

    fun setLastTimeResetLogcat(time: Long) {
        getPref(context).edit {
            putLong(LAST_TIME_RESET_LOGCAT, time)
            apply()
        }
    }


}