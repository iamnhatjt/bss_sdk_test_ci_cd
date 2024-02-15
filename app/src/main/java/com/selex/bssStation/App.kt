package com.selex.bssStation

import android.annotation.SuppressLint
import android.app.Application
import android.app.DownloadManager
import android.content.Context
import android.content.IntentFilter
import android.os.Build
import android.os.Environment
import android.telephony.CellSignalStrengthLte
import android.telephony.SignalStrength
import android.telephony.TelephonyCallback
import android.telephony.TelephonyManager
import android.util.Log
import androidx.core.content.ContextCompat
import com.selex.bssStation.core.utils.DownloadReceiver
import com.selex.bssStation.core.utils.TAG
import com.selex.bssStation.service.BackgroundService
import dagger.hilt.android.HiltAndroidApp
import java.io.File
import kotlin.properties.Delegates

@HiltAndroidApp
class App : Application() {
    @SuppressLint("UnspecifiedRegisterReceiverFlag")
    override fun onCreate() {
        super.onCreate()

        instance = this
        context = applicationContext
        registerReceiver(DownloadReceiver(), IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE))
        catchUncaughtException()
        listenLte()
        deleteFileApk()
    }

    private fun listenLte() {
        val telephonyManager = getSystemService(TelephonyManager::class.java)

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            // Register the listener for the telephony manager
            val listener = object : TelephonyCallback(), TelephonyCallback.SignalStrengthsListener {
                override fun onSignalStrengthsChanged(signalStrength: SignalStrength) {
                    try {
                        Log.d(TAG, "RSSI: $signalStrength")
                        Log.d(TAG, "RSSI: ${signalStrength.cellSignalStrengths}")

                        val strengthLte = signalStrength.cellSignalStrengths[0] as CellSignalStrengthLte
                        // RSSI in range [-113;-51] (dBm)
                        if(strengthLte.rssi in -113..-51) {
                            BackgroundService.rssi = strengthLte.rssi
                        }
                        Log.d(TAG, "RSSI change: ${BackgroundService.rssi}")
                    } catch (e: Exception) {
                        Log.e(TAG, "RSSI Exception: ${e.message}")
                    }
                }
            }
            telephonyManager.registerTelephonyCallback(ContextCompat.getMainExecutor(applicationContext), listener)
        }
    }

    companion object {
        var publishRestartCompleteToBE: Boolean = false
        var isUpdatingApp = false

        var context: Context by Delegates.notNull()
            private set

        lateinit var instance: Application
        init {
            System.loadLibrary("bss_sdk_wrapper")
        }
    }

    private fun catchUncaughtException() {
        Thread.setDefaultUncaughtExceptionHandler { thread, throwable ->
            Log.e(TAG, "ERROR UNCAUGHT: ${throwable.message} in thread: ${thread.name}")
            throwable.printStackTrace()
        }
    }

    private fun deleteFileApk() {
        Log.d(TAG, "Deleting All file apk...")
        try {
            val dirDownload = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
            if(!dirDownload.exists()) {
                return
            }

            val apkFolder = File(dirDownload, "app")
            if(!apkFolder.exists()) {
                return
            }

            deleteRecursive(apkFolder)
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    private fun deleteRecursive(fileOrDirectory: File) {
        if (fileOrDirectory.isDirectory) {
            val childFiles = fileOrDirectory.listFiles()
            if(childFiles != null) {
                for (childFile in childFiles) {
                    deleteRecursive(childFile)
                }
            }
        }
        Log.d(TAG, "Deleting ${fileOrDirectory.absolutePath}")
        fileOrDirectory.delete()
    }

}