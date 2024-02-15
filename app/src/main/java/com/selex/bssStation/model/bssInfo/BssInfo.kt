package com.selex.bssStation.model.bssInfo

data class CabinetMqttToBEModel(
    val state: State?
)

data class State(
    val reported: ReportedModel?
)

data class ReportedModel(
    val cabinet: CabinetModel?,
    val device_type: String?,
    val factory_date: String?,
    val fw_version: String?,
    val hw_version: String?,
    val lot_number: String?,
    val model: String?,
    val sn: String?,
    val state: Int?,
    val charger: ChargerMapModel,
    val electric_meter: ElectricModel?,
    val lte: LTEModel?,
    val total_pack: Int?,
    val sys: SystemInformation,
    val actual_bp: Map<Int, Any>?
)

data class ElectricModel(
    val cos: Float?,
    val current: Float?,
    val kwh: Float?,
    val freq: Float?,
    val voltage: Float?
)

data class LTEModel(
    val esim: String,
    val sim: String,
    val band: String,
    val rssi: Int?
)
data class ChargerMapModel(
     val number: Int?,
     val chargers: HashMap<Int, ChargerModel>?,
)
data class ChargerModel(
   // val number: Int?,
   // val cab_list: Int?,
    val cur: Int?,
    val vol: Int?,
    val cab: Int?,
    val state: Int?
)

data class CabinetModel(
    val cabinets: HashMap<Int, X1Model>?,
    val number: Int?,
    val ready_pack: Int?,
    val total_pack: Int?,
    var total_not_connect: Int?,
)

data class X1Model(
    val battery: BatteryModel?,
    val connected: Int?,
    val door_state: Int?,
    val fan_state: Int?,
    val op_state: Int?,
    val state: Int?,
    val temp: IntArray,
    val temp_threshold: Int?,
    val charged: Int
)

data class BatteryModel(
    val sn: String?,
    val soc: Int?
)

data class DesiredState(
    val cabinet: CabinetModel?,

    )

data class BssInfo(
    val desiredState: DesiredState?,
)

data class SystemInformation(
    val cpu_temp: Float,
    val mem_total: Int,
    val mem_used: Int,
    val mem_free: Int,
    val mem_percent: Int
)
