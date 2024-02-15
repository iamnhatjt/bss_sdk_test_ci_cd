package com.selex.bssStation.model.bssInfo

data class BPState(
    val state: BPReported
)
data class BPReported(
    val reported: BPBattery
)
data class BPBattery(
    val batteries: Array<BPModel>
) {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as BPBattery

        if (!batteries.contentEquals(other.batteries)) return false

        return true
    }

    override fun hashCode(): Int {
        return batteries.contentHashCode()
    }
}

data class BPModel(
    val sn: String?,
    val device_type: String = "bp",
    val fw_version: String?,
    val soc: Int?,
    val soh: Int?,
    val cycle: Int?,
    val vol: Int?,
    val cur: Int?,
    val op_state: Int?,
    val status: Int?,
    val temps: IntArray?,
    val cells_vol: IntArray?,
    val assigned: BPAssigned,
    val slot: Int?
) {
    constructor(sn: String, fw_version: String?, soc: Int?, soh: Int?, cycle: Int?, vol: Int?, cur: Int?, op_state: Int?, status: Int?, temps: IntArray?, cells_vol: IntArray?, assigned: BPAssigned, slot: Int): this(sn, "bp", fw_version, soc,  soh,cycle, vol, cur, op_state, status, temps, cells_vol, assigned, slot ) {
    }

}

data class BPAssigned(
    val sn: String?,
    val type: String?
)


//assigned
//assigned.sn
//assigned.type

//
//temps[7]
//cells_vol[16]