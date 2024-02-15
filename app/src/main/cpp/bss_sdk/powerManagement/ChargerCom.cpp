//
// Created by vnbk on 24/07/2023.
//

#include "ChargerCom.h"
#include "Logger.h"

#define TAG "ChargerCom"

ChargerCom::~ChargerCom() noexcept {

}

int32_t ChargerCom::charged(const uint32_t& _id) {
    /// TODO: Control charger over communication charger
    return 0;
}

int32_t ChargerCom::setConfig(const uint32_t &_id, bool _isAuto, const uint32_t &_cur, const uint32_t &_vol) {
    /// TODO: control charger include current, voltage...
    return 0;
}

int32_t ChargerCom::discharged(const uint32_t& _id) {
    /// TODO: Control charger over communication charger
    return 0;
}

int32_t ChargerCom::getChargerCur(const uint32_t& _id) {
    /// TODO: Control charger over communication charger
    return 0;
}

int32_t ChargerCom::getChargerVol(const uint32_t& _id) {
    /// TODO: Control charger over communication charger
    return 0;
}

int32_t ChargerCom::getState(const uint32_t &_id) {
    /// TODO: Get charger state
    return 0;
}

void ChargerCom::process() {
    /// TODO: Process Charger Protocol
}