package com.selex.bssStation.core.utils

import android.app.DownloadManager
import android.content.Context
import android.net.Uri
import android.os.Environment
import android.util.Log
import com.selex.bssStation.R
import java.io.File

class ApkDownloader(
    private val context: Context,
) {

    fun downLoadFileFromUrl(url: String, folder: String, filename: String): Long {
        Log.d(TAG, "${javaClass.simpleName}: Downloading file with url: $url, folder: $folder, filename: $filename")
        val downloadManager = context.getSystemService(DownloadManager::class.java)
        val uri = Uri.parse(url)
        // download location is Download folder
        val dirDownload = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
        if(!dirDownload.exists()) {
            dirDownload.mkdir()
        }

        val downloadFolder = File(dirDownload, folder)
        if(!downloadFolder.exists()) {
            downloadFolder.mkdir()
        }

        val downloadLocation = File(downloadFolder, filename)

        // start download manager
        val request = DownloadManager.Request(uri)
            .setTitle(filename)
            .setDescription(context.getString(R.string.downloading))
            .setAllowedNetworkTypes(DownloadManager.Request.NETWORK_MOBILE or DownloadManager.Request.NETWORK_WIFI)
            .setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED)
            .setDestinationUri(Uri.fromFile(downloadLocation))
        return downloadManager.enqueue(request)
    }

}