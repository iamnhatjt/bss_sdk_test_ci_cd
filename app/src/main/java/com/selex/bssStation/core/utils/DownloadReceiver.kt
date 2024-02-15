package com.selex.bssStation.core.utils

import android.app.DownloadManager
import android.content.BroadcastReceiver
import android.content.ContentResolver
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.os.CountDownTimer
import android.provider.MediaStore
import android.util.Log
import com.selex.bssStation.App
import com.selex.bssStation.screen.Screen
import com.selex.bssStation.screen.ScreenManager
import com.selex.bssStation.service.BackgroundService
import com.selex.bssStation.service.MessageEvent
import org.greenrobot.eventbus.EventBus
import java.io.File
import java.util.LinkedList
import java.util.Queue

class DownloadReceiver : BroadcastReceiver() {

    override fun onReceive(context: Context, intent: Intent) {
        if (intent.action == DownloadManager.ACTION_DOWNLOAD_COMPLETE) {
            BackgroundService.mqttService!!.publish(
                topic = BackgroundService.topicOtaDownloadStatus,
                data = "{" +
                        "  \"message\":\"BSS_DOWNLOAD_COMPLETE\"" +
                        "}"
            )
            try {
                Log.d(TAG, "${javaClass.simpleName}: DOWNLOAD COMPLETE")
                val downloadId = intent.getLongExtra(DownloadManager.EXTRA_DOWNLOAD_ID, -1)
                val downloadManager = context.getSystemService(DownloadManager::class.java)
                val downloadFileUri = downloadManager.getUriForDownloadedFile(downloadId)
                val filePath = getFilePathFromUri(context, downloadFileUri) ?: return
                when (queueDownload.poll()) {
                    Type.BATTERY_PACK -> {}
                    Type.APP -> requestAdminInstallApp(context, filePath)
                    else -> {}
                }
            } catch (e: Exception) {
                BackgroundService.mqttService!!.publish(
                    topic = BackgroundService.topicOtaUpgrade,
                    data = "{" +
                            "  \"error\":\"OTA failed with message: ${e.message}\"" +
                            "}"
                )
            }
        }
    }

    private fun requestAdminInstallApp(
        context: Context,
        filePath: String
    ) {
        object : CountDownTimer(3000, 3000) {
            override fun onTick(p0: Long) {}

            override fun onFinish() {
                if (ScreenManager.currentScreen == Screen.STAND_BY) {
                    Log.d(TAG, "${javaClass.simpleName}: Request admin update with path: $filePath")
                    App.isUpdatingApp = true
                    EventBus.getDefault().post(MessageEvent(message = Constants.BATTERY_STATION_MAINTAINING))
                    BackgroundService.mqttService!!.publish(
                        topic = BackgroundService.topicOtaUpgrade,
                        data = "{" +
                                "  \"message\":\"UPGRADING\"" +
                                "}"
                    )
                    context.sendBroadcast(
                        Intent(ACTION_INSTALL_REQUEST).apply {
                            putExtra("path", filePath)
                        }
                    )
                    object : CountDownTimer(20000, 1000) {
                        override fun onTick(p0: Long) {}

                        override fun onFinish() {
                            App.isUpdatingApp = false
                        }
                    }.start()
                } else {
                    this.start()
                }
            }
        }.start()
    }

    private fun getFilePathFromUri(context: Context, uri: Uri): String? {
        val contentResolver: ContentResolver = context.contentResolver

        // Check if the file scheme is "file"
        if (uri.scheme == "file") {
            return uri.path
        }

        // For Android Q and above, use the content resolver to get the actual file path
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            val cursor = contentResolver.query(uri, null, null, null, null)
            cursor?.use {
                if (it.moveToFirst()) {
                    val columnIndex = it.getColumnIndex(MediaStore.Images.Media.DATA)
                    return it.getString(columnIndex)
                }
            }
        } else {
            // For versions below Android Q, use the FileProvider to get the file path
            val file = File(uri.path.toString())
            return file.absolutePath
        }

        return null
    }

    enum class Type {
        APP, BATTERY_PACK
    }

    companion object {

        private val queueDownload: Queue<Type> = LinkedList()

        fun addQueueDownload(type: Type) {
            queueDownload.offer(type)
        }

    }

}