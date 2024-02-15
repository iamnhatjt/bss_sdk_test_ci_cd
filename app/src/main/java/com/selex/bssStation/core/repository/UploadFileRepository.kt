package com.selex.bssStation.core.repository

import com.selex.bssStation.core.service.UploadFileService
import okhttp3.RequestBody.Companion.toRequestBody
import toMultipartFile
import java.io.File
import javax.inject.Inject

class UploadFileRepository @Inject constructor(
    private val accountService: UploadFileService
) {

    suspend fun uploadFile(filepathInServer: String, file: File) {
        accountService.uploadFile(filepathInServer = filepathInServer.toRequestBody(), file = file.toMultipartFile())
    }

}