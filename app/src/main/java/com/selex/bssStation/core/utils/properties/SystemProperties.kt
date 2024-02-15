package com.selex.bssStation.core.utils.properties

import android.util.Log
import android.view.View
import com.selex.bssStation.core.utility.showSnackbar
import com.selex.bssStation.core.utils.Constants
import java.io.BufferedReader
import java.io.IOException
import java.io.InputStreamReader

object SystemProperties {

    private const val GET_PROP_EXECUTABLE_PATH = "/system/bin/getprop"
    const val SERIAL_NUMBER = "persist.selex.serial"
    const val FACTORY_DATE = "persist.selex.factorydate"
    const val HW_VERSION = "persist.selex.hwversion"
    const val MODEL = "persist.selex.model"
    const val IOT_NUMBER = "persist.selex.iotnumber"

    fun read(propName: String): String {
        var process: Process? = null
        var bufferedReader: BufferedReader? = null
        return try {
            process = ProcessBuilder().command(GET_PROP_EXECUTABLE_PATH, propName)
                .redirectErrorStream(true).start()
            bufferedReader = BufferedReader(InputStreamReader(process.inputStream))
            var line = bufferedReader.readLine()
            if (line == null) {
                line = ""
            }
            line
        } catch (e: Exception) {
            ""
        } finally {
            if (bufferedReader != null) {
                try {
                    bufferedReader.close()
                } catch (e: IOException) {
                    e.printStackTrace()
                }
            }
            process?.destroy()
        }
    }

    fun write(propName: String, value: String, onWriteSuccess: () -> Unit = {}) {
        try {
            ProcessBuilder().command("setprop", propName, value)
                .redirectErrorStream(true).start()
            onWriteSuccess()
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

}