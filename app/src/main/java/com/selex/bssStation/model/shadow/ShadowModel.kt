package com.selex.bssStation.model.shadow

import com.selex.bssStation.model.bssInfo.ChargerModel

data class ShadowModel(
    val state: ShadowStateModel?
)

data class ShadowStateModel(
    val cabinet: ShadowCabinetModel?,
    val type: String?
)

data class ShadowCabinetModel(
    val ready_pack: Int?,
    val cabinets: HashMap<Int, ShadowCabinetItemModel>?,
)

data class ShadowCabinetItemModel(
    val door_state: Int?,
    val fan_state: Int?,
    val type: String?,
)