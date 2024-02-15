package com.selex.bssStation.service

import android.content.Intent
import android.util.Log
import com.selex.bssStation.core.utils.ACTION_REBOOT_REQUEST
import com.selex.bssStation.core.utils.TAG
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.MutableStateFlow
import org.eclipse.paho.client.mqttv3.IMqttActionListener
import org.eclipse.paho.client.mqttv3.IMqttAsyncClient
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken
import org.eclipse.paho.client.mqttv3.IMqttToken
import org.eclipse.paho.client.mqttv3.MqttAsyncClient
import org.eclipse.paho.client.mqttv3.MqttCallbackExtended
import org.eclipse.paho.client.mqttv3.MqttConnectOptions
import org.eclipse.paho.client.mqttv3.MqttMessage
import org.eclipse.paho.client.mqttv3.TimerPingSender
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence

data class SLMqttMessage(
    var topic: String? = null,
    var message: MqttMessage? = null
)

class MqttService {
    private var client: IMqttAsyncClient? = null
    private lateinit var statusFlow: MutableStateFlow<Boolean>
    private lateinit var msgFlow: MutableStateFlow<SLMqttMessage?>
    private val QOS_EXACTLY_ONCE: Int = 0  // There are 3 QoS levels in MQTT: At most once (0) At least once (1) Exactly once (2).
    private var isConnecting: Boolean = true
    private var countConnect = 0
    private lateinit var config: Config
    private val topicPing = "selex/bss/${getSerialNumberBssFromJNI()}/ping"
    private var pingCount = 0
    private var callbackReset: (() -> Unit)? = null
    /**
     * Initialization MqttAsyncClient and setting MqttConnectOptions config
     *
     * @param config
     * @param statusFlow
     * @param msgFlow
     *
     * @return com.selex.bssStation.service.MqttService
     */
    fun init(config: Config, statusFlow: MutableStateFlow<Boolean>, msgFlow: MutableStateFlow<SLMqttMessage?>, callbackReset: (() -> Unit)?): MqttService {
        this.statusFlow = statusFlow
        this.msgFlow = msgFlow
        this.config = config
        this.callbackReset = callbackReset
        client = MqttAsyncClient(
            config.host,
            config.clientId,
            MemoryPersistence(),
            TimerPingSender(),
            null
        )
        setCallbackMQTT()
        options = MqttConnectOptions()
        options.isCleanSession = config.option.cleanSession
        options.connectionTimeout = config.option.timeOut
        options.keepAliveInterval = config.option.keepAliveInterval
        options.isAutomaticReconnect = true
        config.username?.let {
            options.userName = it
        }
        config.password?.let {
            options.password = it.toCharArray()
        }

        return this
    }

    private fun setCallbackMQTT() {
        client?.setCallback(object : MqttCallbackExtended {
            override fun connectComplete(reconnect: Boolean, serverURI: String?) {
                Log.d(TAG, "MQTT Callback: Connect complete(reconnect: $reconnect, serverURI: $serverURI)")
                isConnecting = false
                if (countConnect > 3 || pingCount > 3 ) {
                    countConnect = 0
                    pingCount = 0
                    BackgroundService.subcribeTopicMqtt()
                }
                statusFlow.value = true
            }

            override fun connectionLost(cause: Throwable?) {
                Log.e(TAG, "MQTT Callback: Connect lost")
                CoroutineScope(Dispatchers.IO).launch {
                    this@MqttService.statusFlow.value = false
                }
            }


            override fun messageArrived(topic: String?, message: MqttMessage?) {
                Log.d(TAG, "MQTT Callback:\nTopic: $topic\nMessage: ${String(message?.payload ?: ByteArray(0))}\n ")
                if (topic == topicPing) {
                    pingCount = 0
                }
                message?.let {
                    msgFlow.value = SLMqttMessage(topic, it)
                }
            }

            override fun deliveryComplete(token: IMqttDeliveryToken?) {
                token?.topics?.forEach {
                    Log.d(TAG, "MQTT Callback: Delivered complete on $it")
                }
            }
        })

    }

    fun connect() {
        isConnecting = true
        try {
            Log.d(TAG, "MQTT: Connecting... $options")
            val token = client?.connect(options)
            token?.waitForCompletion()
            Log.d(TAG, "MQTT: Connect success")
        } catch (e: Exception) {
            Log.e(TAG, "MQTT: Connect failed with message: ${e.message}")
            e.printStackTrace()
        }
    }

    fun subscribe(topic: String, qos: Int = 0) {
        try {
            client?.subscribe(topic, qos, null, object : IMqttActionListener {
                override fun onSuccess(asyncActionToken: IMqttToken?) {
                    isConnecting = false
                    Log.d(TAG, "MQTT: Subscribe $topic success")
                }

                override fun onFailure(asyncActionToken: IMqttToken?, exception: Throwable?) {
                    Log.e(TAG, "MQTT: Subscribe $topic failed with message: ${exception?.message}")
                    isConnecting = false
                    if (!isConnecting) {
                        client?.reconnect()
                        setCallbackMQTT()
                    }
                }
            })
        } catch (e: Exception) {
            Log.e(TAG, "MQTT: Subscribe $topic failed with message: ${e.message}")
            e.printStackTrace()
            isConnecting = false
            if (!isConnecting) {
                client?.reconnect()
            }
        }
    }

    fun publish(topic: String, data: String, retain: Boolean = false) {
        if (client?.isConnected == true) {
            try {
                val encodedPayload = data.toByteArray(Charsets.UTF_8)
                val message = MqttMessage(encodedPayload)
                message.qos = QOS_EXACTLY_ONCE
                message.isRetained = retain
                client?.publish(topic, message, null, object : IMqttActionListener {
                    override fun onSuccess(asyncActionToken: IMqttToken?) {
                        Log.d(TAG, "MQTT: published $topic success")
                    }

                    override fun onFailure(asyncActionToken: IMqttToken?, exception: Throwable?) {
                        Log.d(TAG, "Failed to publish $data to $topic")
                    }
                })

            } catch (e: Exception) {
                Log.e(TAG, "MQTT: Publish $topic failed with message: ${e.message}")
                e.printStackTrace()
            }
        } else {
            Log.e(TAG, "MQTT: Publishf faield because not connected")
        }

    }

    fun statusLoop(){
        CoroutineScope(Dispatchers.IO).launch {
            while (true) {
                try {
                    if( client?.isConnected == false) { // try to reconnect if not already trying to connect
                        Log.e(TAG, "MQTT: connect abc")
                        countConnect++
                        if (countConnect < 3) {
                            Log.e(TAG, "MQTT: connect abc reconnect")
                            isConnecting = true
                            client?.reconnect()
                        } else if (countConnect < 20) {
                            isConnecting = true
                            Log.e(TAG, "MQTT: connect abc connect")
                            client?.disconnect()
                            client = MqttAsyncClient(
                                config.host,
                                "clientID${Math.random()}",
                                MemoryPersistence(),
                                TimerPingSender(),
                                null
                            )
                            setCallbackMQTT()
                            client?.connect(options)
                        } else {
                            Log.e(TAG, "MQTT: connect abc reset")
                            callbackReset?.invoke()
                        }
                    } else {
                        if (pingCount > 3) {
                            Log.e(TAG, "MQTT: lost subcribe, reconnect")
                            isConnecting = true
                            this@MqttService.statusFlow.value = false
                            client?.disconnect()
                            client = MqttAsyncClient(
                                config.host,
                                "clientID${Math.random()}",
                                MemoryPersistence(),
                                TimerPingSender(),
                                null
                            )
                            setCallbackMQTT()
                            client?.connect(options)
                            Log.e(TAG, "MQTT: lost subcribe, reconnect")
                        } else {

                            pingCount++
                            publish(
                                topic = topicPing,
                                data = "{\"type\": \"PING\"}"
                            )
                            Log.e(TAG, "MQTT: still ok $pingCount")
                        }

                    }
                } catch(e: Exception){
                    println("mqtt: error in MQTT status loop")
                    println(e)
                }
                delay(300000L)
            }
        }
    }

    companion object {
        @Volatile
        private var mqttService: MqttService? = null
        private lateinit var options: MqttConnectOptions
        fun init(): MqttService = mqttService ?: synchronized(this) {
            mqttService ?: MqttService().apply {
                mqttService = this
            }
        }
    }
}