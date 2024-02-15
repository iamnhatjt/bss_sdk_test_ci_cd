package com.selex.bssStation.core.utils

import android.content.Intent
import com.selex.bssStation.screen.base.BaseActivity
import com.selex.bssStation.screen.standBy.ScreenStandbyActivity
import com.selex.bssStation.service.BackgroundService
import com.selex.bssStation.service.NoActionCommon


fun logout(ctx: BaseActivity, needSyncData: Boolean = true) {
    ctx.sharedPreferences.setToken("")
    BackgroundService.isNeedSyncData = true
    if (needSyncData) {
        BackgroundService.syncData()
    }
    NoActionCommon.instance.cancelTimer()
    val intent = Intent(ctx, ScreenStandbyActivity::class.java)
    intent.flags = Intent.FLAG_ACTIVITY_CLEAR_TOP
//    intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK
//    intent.flags = Intent.FLAG_ACTIVITY_CLEAR_TASK

    ctx.startActivity(intent)
    ctx.finish()
}
