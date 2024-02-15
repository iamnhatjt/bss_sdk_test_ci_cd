package com.selex.bssStation.service

import android.app.ActivityManager
import android.app.Service
import android.content.Intent
import android.os.Build
import android.os.Handler
import android.os.IBinder
import android.util.Log
import com.google.gson.Gson
import com.google.gson.JsonSyntaxException
import com.selex.bssStation.App
import com.selex.bssStation.App.Companion.context
import com.selex.bssStation.BuildConfig
import com.selex.bssStation.core.repository.UploadFileRepository
import com.selex.bssStation.core.utils.*
import com.selex.bssStation.model.bss.BssStatusModel
import com.selex.bssStation.model.bssInfo.*
import com.selex.bssStation.model.ota.app.OTAModel
import com.selex.bssStation.model.shadow.ShadowModel
import com.selex.bssStation.screen.Screen
import com.selex.bssStation.screen.ScreenManager
import dagger.hilt.android.AndroidEntryPoint
import formatFullTime
import getIndexFromBE
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.launch
import org.greenrobot.eventbus.EventBus
import org.json.JSONObject
import setIndexCabinetToBE
import java.io.BufferedReader
import java.io.File
import java.io.FileFilter
import java.io.InputStreamReader
import java.io.RandomAccessFile
import java.util.Calendar
import java.util.TimeZone
import java.util.regex.Pattern
import javax.inject.Inject

@AndroidEntryPoint
class BackgroundService: Service() {

    @Inject
    lateinit var uploadFileRepository: UploadFileRepository

    @Inject
    lateinit var sharedPreferences: SharedPreferences

    private var job: Job? = null
    private var logcatJob: Job? = null
    private val scope = MainScope()

    private val msgFlow: MutableStateFlow<SLMqttMessage?> = MutableStateFlow(null)
    private val statusFlow = MutableStateFlow(false)



    override fun onBind(p0: Intent?): IBinder? = null

    companion object {
        var mqttService: MqttService? = null
        var bss: BssStatusModel? = null
        var rssi: Int? = null
        var misi: String = ""
        var isNeedSyncData = false
        var resultBatteryConnected: String? = null
        var resultDoorChange: JSONObject? = null
        private val serialNumber = getSerialNumberBssFromJNI()

        private val topicSelexSubscribeOne = "selex/bss/$serialNumber/notification" // những thay đổi riêng của trạm (status)
        private val topicSelexSubscribeTwo = "selex/bss/notification" // những thay đổi chung của trạm (banner)
        private val topicSelexSubscribeThree = "selex/bss/$serialNumber/shadow/update/delta" // update
        private val topicSelexControlCabinet = "selex/bss/$serialNumber/control"
        private val topicSelexCommandCabinet = "selex/bss/$serialNumber/command"
        private val topicPing = "selex/bss/$serialNumber/ping"
        private val topicOTA = "ota/bss/abs/$serialNumber/temp"
        private val topicLogcat = "selex/bss/$serialNumber/logcat"
        // Publish
        private val topicSelexPublish = "selex/bss/$serialNumber/shadow/update"
        private val topicSelexBpPublic = "selex/bss/$serialNumber/shadow/bulk/bp/update"
        private val topicOtaDownloading = "ota/bss/abs/$serialNumber/downloading"
        val topicOtaDownloadStatus = "ota/bss/abs/$serialNumber/download_status"
        val topicOtaUpgrade = "ota/bss/abs/$serialNumber/upgrading_status"

        private var lostConnectPinCount = 0

        private val activityManager = context.getSystemService(ActivityManager::class.java)
        private fun getTotalBytes(): Long {
            val memoryInfo = ActivityManager.MemoryInfo()
            activityManager.getMemoryInfo(memoryInfo)
            return memoryInfo.totalMem
        }

        private fun getAvailableBytes(): Long {
            val memoryInfo = ActivityManager.MemoryInfo()
            activityManager.getMemoryInfo(memoryInfo)
            return memoryInfo.availMem
        }

        private fun cpuTemperature(): Float? {
            return try {
                val process = Runtime.getRuntime().exec("cat sys/class/thermal/thermal_zone0/temp")
                process.waitFor()
                val reader = BufferedReader(InputStreamReader(process.inputStream))
                val line = reader.readLine()
                if (line != null) {
                    val temp = line.toFloat()
                    temp / 1000.0f
                } else {
                    0f
                }
            } catch (e: Exception) {
                e.printStackTrace()
                0f
            }
        }

        fun bssToBe(): ReportedModel {
            val mapCabinet = HashMap<Int, X1Model>()
            var count = 0
            var countNotCabinet = 0
            for (i in 0 until getNumberCabinetFromJNI()) {
                val battery = BatteryModel(getSerialBattery(i), getSocBatteryFromJNI(i))
                if (getSerialBattery(i)?.isNotEmpty() == true) {
                    count++
                }
                val doorState = getDoorStateFromJNI(i)
                val temptCabinet = getTempCabinetListFromJNI(i);
                if (getStateCabinetFromJNI(i) == 0) {
                    countNotCabinet++
                }
                val x1 = X1Model(
                    battery,
                    getStateCabinetFromJNI(i),
                    doorState,
                    getFanStateFromJNI(i),
                    getOpStateFromJNI(i),
                    null,
                    temptCabinet,
                    getTempCabinetThresholdFromJNI(),
                    getIsChargedFromJNI(i)
                )
                mapCabinet[setIndexCabinetToBE(i)] = x1
            }
            Log.d(TAG, "mapCabinet sync data $mapCabinet")

            val cabinet = CabinetModel(
                mapCabinet,
                getNumberCabinetFromJNI(),
                null,
                count,
                total_not_connect = countNotCabinet
            )

//            val electric = ElectricModel(
//                emeterGetCos(),
//                emeterGetCur(),
//                emeterGetEnergy(),
//                emeterGetFreq(),
//                emeterGetVol()
//            )
            val chargers = HashMap<Int, ChargerModel>()
            for (i in 0 until chargerGetNumberBssJNI()) {
                val charger = ChargerModel(
                    chargerGetCur(i),
                    chargerGetVol(i),
                    setIndexCabinetToBE(chargerGetChargingCab(i)),
                    chargerGetState(i)
                )
                chargers[i] = charger
            }

            val chargerMap = ChargerMapModel(chargerGetNumberBssJNI(), chargers)

            val cpuTemp = cpuTemperature()
            val ramTotal = getTotalBytes()
            val ramAvailable = getAvailableBytes()
            val jsonString = getListBp()
            var map: Map<Int, Any> = HashMap()
            map = Gson().fromJson(jsonString, map.javaClass)
            Log.d(TAG, "BackgroundService: SYNC DATA ... actual bp $map")
            return ReportedModel(
                cabinet,
                "",
                getFactoryDateFromJNI(),
                BuildConfig.VERSION_NAME + " b${BuildConfig.VERSION_CODE}",
                getHwVersionFromJNI(),
                getLotNumberFromJNI(),
                getModelFromJNI(),
                getSerialNumberBssFromJNI(),
                getStateBssFromJNI(),
                chargerMap,
                null,
                LTEModel(
                    esim = "",
                    sim = misi,
                    band = "",
                    rssi = rssi
                ),
                cabinet.total_pack,
                sys = SystemInformation(
                    cpu_temp = cpuTemp ?: 0f,
                    mem_total = (ramTotal / 1000 / 1000).toInt(),
                    mem_used = ((ramTotal - ramAvailable) / 1000 / 1000).toInt(),
                    mem_free = (ramAvailable / 1000 / 1000).toInt(),
                    mem_percent = ((ramTotal - ramAvailable).toFloat() / ramTotal * 100).toInt()
                ),
                actual_bp = map
            )

        }

        fun syncData( myCallback: (() -> Unit)? = null) {
            if (isNeedSyncData) {
                Log.d(TAG, "BackgroundService: SYNC DATA ...")
                val gson = Gson()
                val state = State(bssToBe())
                if  (state.reported?.cabinet?.total_pack == 0 && state.reported?.cabinet?.total_not_connect == getNumberCabinetFromJNI()) {
                    Log.d(TAG, "BackgroundService: SAPCAN SAP CAN SAP CAN $lostConnectPinCount")
                    lostConnectPinCount++
                } else {
                    lostConnectPinCount = 0
                }
                if (lostConnectPinCount == 10) {
                    Log.d(TAG, "BackgroundService: SAP CAN ROI DAY")
                    myCallback?.invoke()
                }
                val cabinetMqttToBEModel = CabinetMqttToBEModel(state)
                val jsonValue = gson.toJson(cabinetMqttToBEModel)
                mqttService?.publish(topicSelexPublish, jsonValue)
            }
        }

        fun syncBP() {
            Log.d(TAG, "BackgroundService: SYNC DATA BP ...")
            var batteries = emptyArray<BPModel>()
            for (i in 0 until getNumberCabinetFromJNI()) {
                val serial = getSerialBattery(i)
                if (serial?.isNotEmpty() == true) {
                    try {
                        val bpInfoJson = getInfoBp(i)
                        val obj = JSONObject(bpInfoJson.toString())
                        val soc = obj["soc"] as? Int ?: 0
                        val soh = obj["soh"] as? Int ?: 0
                        val cycle = obj["cycle"] as? Int ?: 0
                        val status = obj["status"] as? Int ?: 0
                        val version = obj["version"] as? String ?: ""
                        val vol = obj["vol"] as? Int ?: 0
                        val cur = obj["cur"] as? Int ?: 0
                        val opState = obj["op_state"] as? Int ?: 0
                        val temp = getTempBp(i)
                        val cellTemp = getCellVol(i)
                        val sn = getAssignedSn(i)
                        val assigned = BPAssigned(sn, "bss")
                        val battery = BPModel(
                            sn = serial,
                            fw_version = version,
                            soc = soc,
                            soh = soh,
                            cycle = cycle,
                            vol = vol,
                            cur = cur,
                            op_state = opState,
                            status = status,
                            temps = temp,
                            cells_vol = cellTemp,
                            assigned = assigned,
                            slot = setIndexCabinetToBE((i))
                        )
                        batteries += battery
                    } catch (e: Exception) {
                        e.printStackTrace();
                    }

                }
            }
            val bp = BPBattery(batteries)
            val reported = BPState(BPReported(bp))
            val gson = Gson()
            val jsonValue = gson.toJson(reported)
            mqttService?.publish(topicSelexBpPublic, jsonValue)
        }

       fun subcribeTopicMqtt() {
           Log.d(TAG, "subcribeTopicMqtt")
           mqttService!!.subscribe(topicPing)
           mqttService!!.subscribe(topicSelexControlCabinet)
           mqttService!!.subscribe(topicSelexSubscribeOne)
           mqttService!!.subscribe(topicSelexSubscribeTwo)
           mqttService!!.subscribe(topicSelexSubscribeThree)
           mqttService!!.subscribe(topicSelexCommandCabinet)
           mqttService!!.subscribe(topicOTA)
           mqttService!!.subscribe(topicLogcat)
        }
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        //MqttManager.connectBroker(this)
        bssInitFromJNI()

        Handler().postDelayed({
            val run: Runnable = ThreadProcessBss()
            val thread = Thread(run, "My new thread")
            thread.start()

        }, 3000)
        initMQtt()
        startSyncData()
        saveAllLogcatIntoFile()
        return START_STICKY
    }

    private fun initMQtt() {
        mqttService = MqttService.init()
        val config = Config(
            host = Constants.HOST_SELEX,
            clientId = "clientID${Math.random()}",
            option = Config.Option(
                cleanSession = true,
                automaticReconnect = true,
                timeOut = 20,
                keepAliveInterval = 10,
                username = Constants.USERNAME_SELEX,
                password = Constants.PASSWORD_SELEX
            )
        )

        mqttService!!.init(config, statusFlow, msgFlow, callbackReset = {
            sendBroadcast(Intent(ACTION_REBOOT_REQUEST))
        }).connect()
        mqttService?.statusLoop()
        CoroutineScope(Dispatchers.IO).launch {
            statusFlow.collect { isConnected ->
                Log.d(TAG, "MQTT: Collect statusFlow: $isConnected")
                if (isConnected) {
                    if (!App.publishRestartCompleteToBE) {
                        mqttService!!.publish(
                            topicSelexSubscribeOne,
                            "{" +
                                    "\"type\":\"${Constants.BSS_RESTART_COMPLETE}\"," +
                                    "\"value\":\"1\"," +
                                    "\"ts\":${System.currentTimeMillis()}" +
                                    "}"
                        )
                        App.publishRestartCompleteToBE = true
                    }
                  subcribeTopicMqtt()
                }
            }
        }

        CoroutineScope(Dispatchers.IO).launch {
            msgFlow.collect {
                Log.d(TAG, "MQTT: Collect msgFlow:\nTopic: ${it?.topic}\nMessage: ${it?.message.toString()}\n ")
                it?.let { msg ->
                    try {
                        val jsonMsg = JSONObject(msg.message.toString())
                        when (msg.topic) {
                            topicPing -> {
                                try {
                                    val type = jsonMsg.getString("type")
                                    if(type != "PING_REQUEST") {
                                        return@let
                                    }

                                    mqttService!!.publish(
                                        topic = topicPing,
                                        data = "{\"type\": \"PING_RESPONSE\"}"
                                    )
                                } catch (e: Exception) {
                                    mqttService!!.publish(
                                        topic = topicPing,
                                        data = "{\"type\": \"PING_RESPONSE_ERROR: ${e.message}\"}"
                                    )
                                }
                            }

                            topicSelexSubscribeOne, topicSelexSubscribeTwo -> {
                                val type = jsonMsg.getString("type")
                                if (type == Constants.BSS_RESTART || type == "BSS_RESTART_FORCE") {
                                    if (ScreenManager.currentScreen != Screen.STAND_BY && type != "BSS_RESTART_FORCE") {
                                        mqttService!!.publish(
                                            topicSelexSubscribeOne,
                                            "{" +
                                                    "\"type\":\"${Constants.BSS_BUSY}\"," +
                                                    "\"ts\":${System.currentTimeMillis()}\"" +
                                                    "}"
                                        )
                                    } else {
                                        sendBroadcast(Intent(ACTION_REBOOT_REQUEST))
                                    }
                                } else {
                                    val event = MessageEvent(type)
                                    EventBus.getDefault().post(event)
                                }
                            }

                            topicSelexSubscribeThree -> {
                                try {
                                    val shadow = Gson().fromJson(
                                        msg.message.toString(),
                                        ShadowModel::class.java
                                    )
                                    val event =
                                        MessageEvent("NUMBER_PIN:${shadow.state?.cabinet?.ready_pack ?: 0}")
                                    EventBus.getDefault().post(event)
                                } catch (e: Exception) {
                                    Log.e(TAG, "BackgroundService: PARSE ERROR $msg")
                                    e.printStackTrace()
                                }
                            }

                            topicSelexControlCabinet -> {
                                if (jsonMsg.has("openDoor") && jsonMsg.has("id")) {
                                    val id: Int = getIndexFromBE(jsonMsg["id"] as Int)
                                    setDoorOpen(id)
                                    Log.d(TAG, "setDoorOpenFromJNI: by control")
                                } else if (jsonMsg.has("charge") && jsonMsg.has("id")) {
                                    val id: Int = getIndexFromBE(jsonMsg["id"] as Int)
                                    Log.d(TAG, "chargerCharge: by control")
                                    chargerCharge(id)
                                } else if (jsonMsg.has("disCharge") && jsonMsg.has("id")) {
                                    val id: Int = getIndexFromBE(jsonMsg["id"] as Int)
                                    Log.d(TAG, "chargerDischarge: by control")
                                    chargerDischarge(id)
                                }
                            }

                            topicOTA -> {
                                try {
                                    if(ScreenManager.currentScreen != Screen.STAND_BY) {
                                        mqttService!!.publish(
                                            topic = topicOtaDownloading,
                                            data = "{" +
                                                    "  \"message\":\"BSS_BUSY\"" +
                                                    "}"
                                        )
                                    } else {
                                        Gson().fromJson(msg.message.toString(), OTAModel::class.java)
                                            ?.run {
                                                val versionCode = BuildConfig.VERSION_CODE
                                                if (build > versionCode) {
                                                    DownloadReceiver.addQueueDownload(DownloadReceiver.Type.APP)
                                                    ApkDownloader(context).downLoadFileFromUrl(
                                                        url = url,
                                                        folder = "app",
                                                        filename = "selex_bss_${System.currentTimeMillis()}.apk"
                                                    )
                                                    mqttService!!.publish(
                                                        topicOtaDownloading,
                                                        data = "{" +
                                                                "  \"message\":\"BSS_DOWNLOADING\"" +
                                                                "}"
                                                    )
                                                }
                                            }
                                    }
                                } catch (e: Exception) {
                                    e.printStackTrace()
                                }
                            }

                            topicSelexCommandCabinet -> {
                                try {
                                    val shadow = Gson().fromJson(
                                        msg.message.toString(),
                                        ShadowModel::class.java
                                    )
                                    if (shadow.state?.type == Constants.BSS_RESTART) {
                                        if (ScreenManager.currentScreen != Screen.STAND_BY) {
                                            mqttService!!.publish(
                                                topicSelexSubscribeOne,
                                                "{" +
                                                        "\"type\":\"${Constants.BSS_BUSY}\"," +
                                                        "\"ts\":${System.currentTimeMillis()}\"" +
                                                        "}"
                                            )
                                        } else {
                                            sendBroadcast(Intent(ACTION_REBOOT_REQUEST))
                                        }
                                    }


                                    if (shadow.state?.cabinet?.cabinets?.isNotEmpty() == true) {
                                        val cabinets = shadow.state.cabinet.cabinets
                                        val key = cabinets.keys.first()
                                        if (cabinets.values.first().door_state != null) {
                                            val value = cabinets.values.first().door_state
                                            if (value == 1) {
                                                Log.d(TAG, "setDoorOpenFromJNI: by dashboard")
                                                setDoorOpen(getIndexFromBE(key))
                                            }
                                        }

                                        if (cabinets.values.first().type != null) {
                                            val type = cabinets.values.first().type
                                            if (type == "charge") {
                                                chargerCharge(getIndexFromBE(key))
                                            }
                                            if (type == "discharge") {
                                                chargerDischarge(getIndexFromBE(key))
                                            }
                                        }

                                    }
                                } catch (e: JsonSyntaxException) {
                                    Log.e(TAG, "BackgroundService: PARSE ERROR $msg")
                                    e.printStackTrace()
                                }
                            }

                            topicLogcat -> {
                                logcatJob?.cancel()
                                logcatJob = CoroutineScope(Dispatchers.IO).launch logcatJob@ {
                                    val type = jsonMsg.getString("type") ?: return@logcatJob


                                    val file = File(filesDir, "all_logcat.txt")

                                    if (type != "REQUEST" && type != "REQUEST_INFO"&& type != "REQUEST_CLEAR") {
                                        return@logcatJob
                                    }
                                    if(type == "REQUEST_CLEAR") {
                                        file.delete()
                                        file.createNewFile()
                                        Log.d(TAG, "----------------- CLEAR LOG CAT by MQTT -----------------")
                                        mqttService!!.publish(
                                            topic = topicLogcat,
                                            data = "{\n" +
                                                    "  \"type\": \"RESPONSE\",\n" +
                                                    "  \"info\": \"Clear file logcat success\"\n" +
                                                    "}"
                                        )
                                        return@logcatJob
                                    }

                                    var lines: List<String> = emptyList()
                                    try {
                                        lines = file.bufferedReader().readLines()
                                    } catch (e: Exception) {
                                        mqttService!!.publish(
                                            topicLogcat,
                                            data = "{\n" +
                                                    "  \"type\": \"RESPONSE\",\n" +
                                                    "  \"error\": \"Error: ${e.message}\"\n" +
                                                    "}"
                                        )
                                        return@logcatJob
                                    }


                                    // Return info
                                    if (type == "REQUEST_INFO") {
                                        mqttService!!.publish(
                                            topicLogcat,
                                            data = "{\n" +
                                                    "  \"type\": \"RESPONSE\",\n" +
                                                    "  \"info\": \"File logcat: ${String.format("%.2f", file.length() / (1024f * 1024f))}MB, ${lines.size} lines\"\n" +
                                                    "}"
                                        )
                                        return@logcatJob
                                    }

                                    Log.d(TAG, "Pushing file logcat ...")
                                    val filepath = "$serialNumber/logcat_${formatFullTime(System.currentTimeMillis())}.txt"
                                    val tempFile = File(filesDir, "logcat.txt")
                                    if (tempFile.exists()) {
                                        tempFile.delete()
                                    }
                                    tempFile.createNewFile()
                                    try {
                                        if (jsonMsg.has("start") && jsonMsg.has("end")) {
                                            val start = jsonMsg.getInt("start")
                                            val end = jsonMsg.getInt("end")
                                            lines = lines.subList(start, end)
                                        }
                                        lines.forEach { line ->
                                            tempFile.appendText("$line\n")
                                        }
                                        mqttService!!.publish(
                                            topic = topicLogcat,
                                            data = "{\n" +
                                                    "  \"type\": \"RESPONSE\",\n" +
                                                    "  \"status\": \"Pushing file ${String.format("%.2f", tempFile.length() / (1024f * 1024f))}MB, ${lines.size} lines\n" +
                                                    "}"
                                        )
                                        uploadFileRepository.uploadFile(filepath, tempFile)
                                        mqttService!!.publish(
                                            topic = topicLogcat,
                                            data = "{\n" +
                                                    "  \"type\": \"RESPONSE\",\n" +
                                                    "  \"link\": \"${Constants.FILE_URL}/$filepath\"\n" +
                                                    "}"
                                        )
                                    } catch (e: Exception) {
                                        e.printStackTrace()
                                        mqttService!!.publish(
                                            topic = topicLogcat,
                                            data = "{\n" +
                                                    "  \"type\": \"RESPONSE\",\n" +
                                                    "  \"error\": \"Push file (${String.format("%.2f", tempFile.length() / (1024f * 1024f))}MB, ${lines.size} lines) failed with message: ${e.message}\"\n" +
                                                    "}"
                                        )
                                    }
                                }
                            }

                        }
                    } catch (e: Exception) {
                        Log.e(TAG, "Exception: ${e.message}")
                        e.printStackTrace()
                    }
                }
            }
        }
    }

    private fun saveAllLogcatIntoFile() {
        // Run command get logcat
        val command = "logcat"
        val process = ProcessBuilder()
            .command(command)
            .redirectErrorStream(true)
            .start()

        // Write logcat into file
        val file = File(filesDir, "all_logcat.txt")
        CoroutineScope(Dispatchers.IO).launch {
            process.inputStream.bufferedReader().forEachLine { line ->

                // Reset logcat file at 2AM
                val currentTime = System.currentTimeMillis()
                val calendar = Calendar.getInstance().apply {
                    timeZone = TimeZone.getTimeZone("Asia/Ho_Chi_Minh")
                    timeInMillis = currentTime
                }
                if (
                    calendar.get(Calendar.HOUR_OF_DAY) in 1..2 &&
                    currentTime - sharedPreferences.getLastTimeResetLogcat() > 12 * 60 * 60 * 1000L
                ) {
                    file.delete()
                    file.createNewFile()
                    Log.d(TAG, "CLEAR LOG CAT -----------")
                    sharedPreferences.setLastTimeResetLogcat(currentTime)
                }
                if (!file.exists()) {
                    file.createNewFile()
                }
                file.appendText("$line\n")
            }
        }
    }

    private fun startSyncData() {
        cancelTimer()
        job = scope.launch {
            while (true) {
                handleGetInfoBss()
                delay(60000)
            }
        }
    }

    private fun cancelTimer() {
        job?.cancel()
        job = null
    }

    private fun handleGetInfoBss() {
        if (isNeedSyncData) {
            syncData(myCallback = {

                sendBroadcast(Intent(ACTION_REBOOT_REQUEST))
            })
            syncBP()
        }
    }

    /**
     * Call back from JNI
     * */
    fun callBackBssFromJNI(id: Int, eventType: Int, eventData: String) {
        if (eventType == 5) {//Bp is connected to system
            Log.d(TAG, "DEVICE Callback: Bp is disconnected to system: $eventData")
            val event = MessageEvent(Constants.BATTERY_DISCONNECT)
            EventBus.getDefault().post(event)
            handleGetInfoBss()
        } else if (eventType == 0) {//Door status is changed
            Log.d(TAG, "DEVICE Callback: Cabinet door status is changed: $eventData")
            val event = MessageEvent(Constants.BSS_DOOR_CHANGE, eventData)
            val obj = JSONObject(eventData)
            resultDoorChange = obj
            EventBus.getDefault().post(event)
            handleGetInfoBss()
        } else if (eventType == 3) {//Cabinet try read bp info
            //         Log.d(TAG, "callBackBssFromJNI Cabinet try read bp info: $eventData")
        } else if (eventType == 4) {//Cabinet BSS_BP_CONNECTED
            Log.d(TAG, "DEVICE Callback: Cabinet battery connected: $eventData")
            val event = MessageEvent(Constants.BATTERY_CONNECTED, eventData)
            resultBatteryConnected = eventData
            EventBus.getDefault().post(event)
            handleGetInfoBss()
        } else if (eventType == 7) {//assign sn battery
            Log.d(TAG, "DEVICE Callback: Cabinet battery assign success: $eventData")
            val event = MessageEvent(Constants.ASSIGN_BATTERY_SUCCESS, eventData)
            EventBus.getDefault().post(event)
        }
    }

    external fun bssInitFromJNI()

}