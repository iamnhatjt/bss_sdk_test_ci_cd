//
// Created by vnbk on 24/07/2023.
//

#include "ChargerManager.h"

#include <utility>
#include <algorithm>
#include "Logger.h"

#define TAG "ChargerManager"

void ChargerManager::bpCmdCallback(BP_COMMAND _cmd, int32_t _isSuccess, const uint32_t &_cabId, void *_data, void *_arg) {
    (void*)_data;
    if(_cmd == BP_COMMAND::BP_CMD_CHARGED || _cmd == BP_COMMAND::BP_CMD_DISCHARGED){
        auto manager = (ChargerManager*)_arg;
        auto charger = (ChargerItem*)_data;
        if(!manager || !charger){
            LOG_ERROR(TAG, "Charger Manager is NULL or Charger ID is INVALID");
            return;
        }

        if(charger->m_cabCharging != _cabId){
            LOG_ERROR(TAG, "Cabinet ID that have BP is wrong");
            auto cabItem = charger->m_cabIds.find(_cabId);
            if(cabItem != charger->m_cabIds.end()){
                cabItem->second->m_checkStateTime.reset();
            }
            return;
        }
        std::lock_guard<std::mutex> lockGuard(manager->m_lock);
//        charger->m_cmdWait = false;
        charger->m_cmdSuccess = _isSuccess;
//        charger->m_cmdStep++;

        LOG_INFO(TAG, "Command Charger %d that control BP FET %s", charger->m_chargerId, _isSuccess ? "SUCCESS" : "FAILURE");

        return;
    }
    LOG_ERROR(TAG, "Charger command is Wrong");
}

std::shared_ptr<ChargerManager> ChargerManager::create(std::shared_ptr<ChargerComInterface> _if,
                                                       std::shared_ptr<BpManagerInterface> _bpManager,
                                                       std::shared_ptr<CabManagerInterface> _cabManager,
                                                       const uint8_t& _number) {
    auto chargerManager = std::shared_ptr<ChargerManager>(new ChargerManager(std::move(_if),
                                                                             std::move(_bpManager),
                                                                             std::move(_cabManager),
                                                                             _number));
    if(chargerManager->initialized()){
        LOG_INFO(TAG, "Created ChargerManager SUCCESS");
        return chargerManager;
    }

    LOG_ERROR(TAG, "Created ChargerManager FAILURE");
    return nullptr;
}

ChargerManager::ChargerManager(std::shared_ptr<ChargerComInterface> _if,
                               std::shared_ptr<BpManagerInterface> _bpManager,
                               std::shared_ptr<CabManagerInterface> _cabManager,
                               const uint8_t& _number) :
                                                    m_if(std::move(_if)),
                                                    m_bpManager(std::move(_bpManager)),
                                                    m_cabManager(std::move(_cabManager)),
                                                    m_chargerNumber(_number){

}

ChargerManager::~ChargerManager() noexcept {
    m_if.reset();
    m_cabManager.reset();
    m_bpManager.reset();
}

bool ChargerManager::initialized() {
    if(!m_bpManager || !m_cabManager || !m_chargerNumber){
        return false;
    }

    for (int i = 0; i < m_chargerNumber; ++i) {
        auto charger = std::make_shared<ChargerItem>(i);
        m_chargers.push_back(charger);
    }

    return true;
}

bool ChargerManager::initializedDefault(){
    setCabList(0, LIST_CABS_MANAGED_BY_CHARGER_0_DEFAULT);
    setCabList(1, LIST_CABS_MANAGED_BY_CHARGER_1_DEFAULT);
    return true;
}

int32_t ChargerManager::setCabList(const uint8_t &_chargerId, const std::vector<int32_t> &_cabList) {
    if(_chargerId >= m_chargerNumber){
        LOG_ERROR(TAG, "ID Charger is NOT Valid");
        return -1;
    }
    if(_cabList.empty()){
        LOG_WARN(TAG, "Cabinet list is Empty");
        return -1;
    }

    for(auto& item : _cabList){
        auto chargingCab = std::make_shared<ChargerItem::ChargingCabItem>();
        m_chargers[_chargerId]->m_cabIds.insert(std::pair<int32_t, std::shared_ptr<ChargerItem::ChargingCabItem>>(item, chargingCab));
        LOG_DEBUG(TAG, "Cabinet %d is assigned to charger %d", item, _chargerId);
    }

    if(m_chargers[_chargerId]->m_cabCharging < 0){
        m_chargers[_chargerId]->m_cabCharging = m_chargers[_chargerId]->m_cabIds.begin()->first;
    }
    return 0;
}

std::vector<int32_t> ChargerManager::getCabList(const uint8_t &_chargerId) {
    std::vector<int32_t> cabList;
    if(_chargerId >= m_chargerNumber){
        LOG_ERROR(TAG, "ID Charger is NOT Valid");
        return {};
    }
    for(auto& item : m_chargers[_chargerId]->m_cabIds){
        cabList.push_back(item.first);
    }
    return cabList;
}

int32_t ChargerManager::enableCab(const uint32_t &_cabId, bool _enable) {
    for(auto& charger : m_chargers){
        auto item = charger->m_cabIds.find((int32_t)_cabId);
        if(item != charger->m_cabIds.end()){
            if(_enable) {
                item->second->m_chargingCabState = CHARGING_CAB_STATE::CHARGING_CAB_ENABLE;
            }
            else{
                item->second->m_chargingCabState = CHARGING_CAB_STATE::CHARGING_CAB_DISABLE;
                if(charger->m_state == CHARGER_STATE::CHARGER_STATE_CHARGED && charger->m_cabCharging == _cabId){
                    startCmd(charger, _cabId, CHARGER_CMD::CHARGER_CMD_DISCHARGING);
                    return 0;
                }
            }
            LOG_INFO(TAG, "Cabinet %d is %s charging feature", _cabId, _enable ? "ENABLE" : "DISABLE");
            return 0;
        }
    }
    LOG_ERROR(TAG, "Cabinet is NOT assigned to the Charger, Please check again");
    return -1;
}

int32_t ChargerManager::setChargerConfig(const uint8_t &_chargerId,
                                         bool _isAuto,
                                         const uint32_t &_cur,
                                         const uint32_t &_vol) {
    if(m_if){
        return m_if->setConfig(_chargerId, _isAuto, _cur, _vol);
    }
    return -1;
}

int32_t ChargerManager::charged(const uint32_t& _cabId, bool _force){
    std::shared_ptr<ChargerItem> charger = nullptr;
    for(auto& item : m_chargers){
        if(item->m_cabIds.find((int32_t)_cabId) != item->m_cabIds.end()){
            charger = item;
            LOG_DEBUG(TAG, "Cabinet %d is managed by Charger %d", charger->m_chargerId, _cabId);
            break;
        }
    }
    if(!charger){
        LOG_ERROR(TAG, "Cabinet %d is NOT under the control of any charger, please use setCabList method", _cabId);
        if(m_observer){
            m_observer->onCharged(-1, _cabId, BSS_RET_FAILURE);
        }
        return -1;
    }

    if(charger->m_state == CHARGER_STATE_CHARGED && charger->m_cabCharging == _cabId){
        LOG_INFO(TAG, "Cabinet %d is charging by charger %d", _cabId, charger->m_chargerId);
        if(m_observer){
            m_observer->onCharged(charger->m_chargerId, _cabId, BSS_RET_SUCCESS);
        }
        return 0;
    }

    if(!_force && charger->m_cabIds.find((int32_t)_cabId)->second->m_chargingCabState == CHARGING_CAB_STATE::CHARGING_CAB_DISABLE){
        LOG_ERROR(TAG, "Cabinet %d is disable for charging", _cabId, charger->m_chargerId);
        if(m_observer){
            m_observer->onCharged(charger->m_chargerId, _cabId, BSS_RET_FAILURE);
        }
        return 0;
    }

    if(checkCommonChargingCondition(_cabId) < 0){
        LOG_ERROR(TAG, "Cabinet %d is NOT enough the charging condition", _cabId);
        if(m_observer){
            m_observer->onCharged(charger->m_chargerId, _cabId, BSS_RET_FAILURE);
        }
        return 0;
    }

    if(_force){
        auto chargingCab = charger->m_cabIds.find((int32_t)_cabId);
        if(chargingCab != charger->m_cabIds.end()){
            auto state = chargingCab->second->m_chargingCabState;
            if(state == CHARGING_CAB_STATE::CHARGING_CAB_DISABLE){
                enableCab(_cabId, true);
            }
            if(state == CHARGING_CAB_STATE::CHARGING_CAB_BLACKLIST){
                chargingCab->second->m_chargingCabState = CHARGING_CAB_STATE::CHARGING_CAB_ENABLE;
                chargingCab->second->m_monitorTime.reset();
            }
        }

        charger->m_chargeForce.m_cabId = (int32_t)_cabId;
        charger->m_chargeForce.m_force = true;

        if(charger->m_dischargeForce.m_force && charger->m_dischargeForce.m_cabId == _cabId){
            charger->m_dischargeForce.m_force = false;
            charger->m_dischargeForce.m_cabId = -1;
        }

        if(charger->m_state == CHARGER_STATE::CHARGER_STATE_CHARGED && charger->m_cabCharging != _cabId){
            startCmd(charger, charger->m_cabCharging, CHARGER_CMD::CHARGER_CMD_DISCHARGING);
            return 0;
        }
    }

    startCmd(charger, _cabId, CHARGER_CMD::CHARGER_CMD_CHARGING);

    return 0;
}

int32_t ChargerManager::discharged(const uint32_t& _cabId, bool _force){
    std::shared_ptr<ChargerItem> charger = nullptr;
    for(auto& item : m_chargers){
        if(item->m_cabIds.find((int32_t)_cabId) != item->m_cabIds.end()){
            charger = item;
            LOG_DEBUG(TAG, "Cabinet %d is managed by Charger %d", charger->m_chargerId, _cabId);
            break;
        }
    }
    if(!charger){
        LOG_ERROR(TAG, "Cabinet %d is NOT under the control of any charger, please use setCabList method", _cabId);
        if(m_observer){
            m_observer->onCharged(-1, _cabId, BSS_RET_FAILURE);
        }
        return -1;
    }

    if(charger->m_state == CHARGER_STATE_DISCHARGED && charger->m_cabCharging == _cabId){
        LOG_INFO(TAG, "Charger %d is already discharged at cabinet %d", charger->m_chargerId, _cabId);
        if(m_observer){
            m_observer->onDischarged(charger->m_chargerId, _cabId, FORCE_DISCHARGED);
        }
        return 0;
    }

    if(charger->m_state == CHARGER_STATE::CHARGER_STATE_CHARGED && charger->m_cabCharging != _cabId){
        LOG_ERROR(TAG, "This cabinet is NOT charging, please check again");
        return -1;
    }

    if(_force){
        charger->m_dischargeForce.m_cabId = (int32_t)_cabId;
        charger->m_dischargeForce.m_force = true;

        LOG_INFO(TAG, "Force discharged at the Cabinet %d", _cabId);

        if(charger->m_chargeForce.m_force && charger->m_chargeForce.m_cabId == _cabId){
            charger->m_chargeForce.m_force = false;
            charger->m_chargeForce.m_cabId = -1;
        }
    }

    startCmd(charger, _cabId, CHARGER_CMD::CHARGER_CMD_DISCHARGING);

    return 0;
}

int32_t ChargerManager::getChargerNumber() {
    return m_chargerNumber;
}

int32_t ChargerManager::getChargerVol(const uint8_t& _chargerId){
    if(m_if){
        return m_if->getChargerVol(_chargerId);
    }
    auto charger = m_chargers[_chargerId];
    if(charger->m_state == CHARGER_STATE_CHARGED){
        auto bp = m_bpManager->getData(charger->m_cabCharging);
        charger->m_vol = bp.m_vol;
        return (int32_t)charger->m_vol;
    }
    return -1;
}

int32_t ChargerManager::getChargerCur(const uint8_t& _chargerId){
    if(m_if){
        return m_if->getChargerCur(_chargerId);
    }
    auto charger = m_chargers[_chargerId];
    if(charger->m_state == CHARGER_STATE_CHARGED){
        auto bp = m_bpManager->getData(charger->m_cabCharging);
        charger->m_cur = bp.m_cur;
        return (int32_t)charger->m_cur;
    }
    return -1;
}

int32_t ChargerManager::getChargerState(const uint8_t& _chargerId){
    if(m_if){
        return m_if->getState(_chargerId);
    }
    auto charger = m_chargers[_chargerId];
    if(charger){
        return charger->m_state;
    }
    return CHARGER_STATE_UNKNOWN;
}

int32_t ChargerManager::getChargingCab(const uint8_t &_chargerId) {
    if(_chargerId >= m_chargerNumber){
        LOG_ERROR(TAG, "Charger ID is INVALID");
        return -1;
    }
    auto charger = m_chargers[_chargerId];

    if(charger->m_state == CHARGER_STATE_CHARGED){
        return charger->m_cabCharging;
    }
    return -1;
}

int32_t ChargerManager::ctlChargerSwitch(const uint32_t &_cabId, uint8_t _value) {
    int count = 0;
    int32_t ret = -1;
    while (count++ < 3){
        ret = m_cabManager->ctlChargerSwitch(_cabId, _value);
        if(ret > 0)
            return ret;
    }
    return -1;
}

void ChargerManager::startCmd(const std::shared_ptr<ChargerItem>& _charger, const uint32_t& _cabId, CHARGER_CMD _cmd){
    _charger->m_cabCharging = (int32_t)_cabId;
    _charger->m_cmdStep = CHARGER_CMD_STEP::CMD_STEP_0;
    _charger->m_cmd = _cmd;
    _charger->m_cmdSuccess = -1;
    _charger->m_cmdWait = false;
    _charger->m_cmdTimeout.reset();
}

void ChargerManager::process() {
    if(m_if){
        m_if->process();
    }

    for (const auto& item : m_chargers) {
        if(item->m_state == CHARGER_STATE_CHARGED && item->m_cmd == CHARGER_CMD::CHARGER_CMD_NONE){
            checkConditionDischarge(item);
            checkBpsInCharging(item);
        }else if(item->m_state == CHARGER_STATE_DISCHARGED && item->m_cmd == CHARGER_CMD::CHARGER_CMD_NONE){
            findCabToCharge(item);
        }
        cmdProcess(item);
    }
}

void ChargerManager::checkBpsInCharging(const std::shared_ptr<ChargerItem>& _charger) {
    if(!_charger){
        return;
    }
    if(_charger->m_state == CHARGER_STATE::CHARGER_STATE_CHARGED){
        for(auto& item : _charger->m_cabIds){
            if(item.first == _charger->m_cabCharging){
                continue;
            }
            if(!m_cabManager->cabHaveBp(item.first)){
                continue;
            }
            auto bp = m_bpManager->getData(item.first);
            if(bp.m_state == BP_STATE_CHARGING && !item.second->m_checkStateTime.getRemainTime()){
                LOG_WARN(TAG, "The BP do NOT choose to charge, but They is still Charging State. It's Dangerous, Turn Off Right NOW !!!");
                ctlChargerSwitch(item.first, CAB_CHARGER_TURN_OFF_STATUS);
                m_bpManager->setCmd(item.first,
                                    BP_COMMAND::BP_CMD_DISCHARGED,
                                    _charger.get(),
                                    bpCmdCallback,
                                    this);
                item.second->m_checkStateTime.reset();
            }
        }
    }
}

void ChargerManager::cmdProcess(const std::shared_ptr<ChargerItem>& _charger) {
    if(_charger->m_cmd == CHARGER_CMD::CHARGER_CMD_NONE){
        return;
    }

    if(!_charger->m_cmdTimeout.getRemainTime()){
        if(_charger->m_cmd == CHARGER_CMD::CHARGER_CMD_DISCHARGING){
            _charger->m_cmdStep = CMD_STEP_0;
            _charger->m_cmdWait = false;
            _charger->m_cmdTimeout.reset();

            LOG_WARN(TAG, "Discharged Command have to execute successfully, Tried again");
            return;
        }
        LOG_WARN(TAG, "Charger current command TIMEOUT, reset current command");
        _charger->cmdReset();
        return;
    }

    int32_t ret = -1;
    BP_COMMAND bpCmd = (_charger->m_cmd == CHARGER_CMD::CHARGER_CMD_CHARGING) ? BP_COMMAND::BP_CMD_CHARGED
                                                                                  : BP_COMMAND::BP_CMD_DISCHARGED;
    uint8_t chargerValue = (_charger->m_cmd == CHARGER_CMD::CHARGER_CMD_CHARGING) ? CAB_CHARGER_TURN_ON_STATUS
                                                                                      : CAB_CHARGER_TURN_OFF_STATUS;

    switch (_charger->m_cmdStep) {
        case CHARGER_CMD_STEP::CMD_STEP_0:
            if (!_charger->m_cmdWait) {
                ret = ctlChargerSwitch(_charger->m_cabCharging, chargerValue);
                if(ret <= 0){
                    LOG_ERROR(TAG, "Could NOT control charger switch on Slave Board. Try again");
                    return;
                }
                LOG_INFO(TAG, "Control charger switch on Slave SUCCESS");

                if(m_cabManager->cabHaveBp(_charger->m_cabCharging)) {
                    m_bpManager->setCmd(_charger->m_cabCharging,
                                        bpCmd,
                                        _charger.get(),
                                        bpCmdCallback,
                                        this);

                    _charger->m_cmdWait = true;
                    _charger->m_cmdTimeout.reset(CHARGER_CMD_TIMEOUT);
                    _charger->m_cmdSuccess = -1;
                }else{
                    _charger->chargerReset();
                }
            }

            if(_charger->m_cmdSuccess < 0){
                return;
            }

            if(_charger->m_cmdSuccess == 0){
                if(_charger->m_cmd == CHARGER_CMD::CHARGER_CMD_DISCHARGING){
                    LOG_WARN(TAG, "Retried turn off switch FET on BP");
                    _charger->m_cmdStep = CHARGER_CMD_STEP::CMD_STEP_0;
                    _charger->m_cmdTimeout.reset(CHARGER_CMD_TIMEOUT);
                    _charger->m_cmdSuccess = -1;
                    return;
                }
                if(_charger->m_cmd == CHARGER_CMD::CHARGER_CMD_CHARGING){
                    if(m_observer) {
                        m_observer->onCharged(_charger->m_chargerId, _charger->m_cabCharging, BSS_RET_FAILURE);
                    }
                    _charger->chargerReset();
                    return;
                }
            }else if(_charger->m_cmdSuccess == 1){
                if(m_if){
                    _charger->m_cmdStep++;
                    return;
                }
                if(_charger->m_cmd == CHARGER_CMD::CHARGER_CMD_CHARGING){
                    _charger->m_state = CHARGER_STATE_CHARGED;
                    _charger->m_currentMonitorTime.reset(CHARGER_CUR_MONITOR_TIME);
                    LOG_WARN(TAG, "Charger %d is charged at cabinet %d", _charger->m_chargerId, _charger->m_cabCharging);

                    if(m_observer) {
                        LOG_WARN(TAG, "onCharged %d at cabinet %d", _charger->m_chargerId, _charger->m_cabCharging);
                        m_observer->onCharged(_charger->m_chargerId, _charger->m_cabCharging, BSS_RET_SUCCESS);
                    }
                    _charger->cmdReset();
                    if(_charger->m_chargeForce.m_force && _charger->m_chargeForce.m_cabId == _charger->m_cabCharging){
                        _charger->m_chargeForce.m_force = false;
                        _charger->m_chargeForce.m_cabId = -1;
                    }
                }else{
                    LOG_INFO(TAG, "Discharged at cabinet %d SUCCESS", _charger->m_cabCharging);
                    auto item = _charger->m_cabIds.find(_charger->m_cabCharging);
                    if(item != _charger->m_cabIds.end()){
                        item->second->m_checkStateTime.reset();
                    }
                    _charger->chargerReset();
                }
            }
            break;
        case CHARGER_CMD_STEP::CMD_STEP_1:
            if(!m_if){
                LOG_ERROR(TAG, "Charger interface is INVALID");
                _charger->chargerReset();
                return;
            }
            if(bpCmd == BP_COMMAND::BP_CMD_CHARGED){
                ret = m_if->charged(_charger->m_chargerId);
                if(ret >= 0){
                    _charger->m_state = CHARGER_STATE_CHARGED;
                    _charger->m_currentMonitorTime.reset(CHARGER_CUR_MONITOR_TIME);
                }
                if (m_observer) {
                    m_observer->onCharged(_charger->m_chargerId, _charger->m_cabCharging, BSS_RET_SUCCESS);
                }
                _charger->cmdReset();
            }else{
                ret = m_if->discharged(_charger->m_chargerId);
                if (ret >= 0 && m_observer) {
                    m_observer->onDischarged(_charger->m_chargerId, _charger->m_cabCharging, FORCE_DISCHARGED);
                }
                _charger->chargerReset();
            }
            break;
        default:
            break;
    }
}

bool ChargerManager::checkBpConditionCharging(const uint32_t &_bpId) {
    auto bpData = m_bpManager->getData(_bpId);

    if(bpData.m_vol >= m_config.m_volStartCharging && bpData.m_soc >= m_config.m_socChargingCondition){
//        LOG_WARN(TAG, "BP is not prioritized for charging because voltage is %d, ", bpData.m_vol);
        return false;
    }

    std::vector<int32_t> cell_temps{bpData.m_temps[0], bpData.m_temps[1],
                                    bpData.m_temps[2], bpData.m_temps[3],
                                    bpData.m_temps[4], bpData.m_temps[5]};

    int32_t cellTempThreshold = m_config.m_cellTempStartCharging;
    if(std::any_of(cell_temps.begin(), cell_temps.end(), [cellTempThreshold](int32_t x){return x >= cellTempThreshold;})){
//        LOG_WARN(TAG, "Cell Temperatures is over threshold, Could NOT charge NOW");
        return false;
    }

    return true;
}

bool ChargerManager::checkTempConditionCharging(const uint32_t &_cabId) {
    auto cab = m_cabManager->getCab(_cabId);
    if(!cab){
        return false;
    }
    std::vector<int32_t> pogoTemps {cab->m_data.m_pogoPinTemp[0], cab->m_data.m_pogoPinTemp[1],
                                    cab->m_data.m_pogoPinTemp[2], cab->m_data.m_pogoPinTemp[3]};

    if(cab->m_data.m_temp >= m_config.m_tempStartCharging){
//        LOG_WARN(TAG, "Cabinet Temperatures is NOT satisfy the condition : %d", cab->m_data.m_temp);
        return false;
    }

    int32_t pogoTempThreshold = m_config.m_pogoTempStartCharging;
    if(std::any_of(pogoTemps.begin(), pogoTemps.end(), [pogoTempThreshold](int32_t x){return x >= pogoTempThreshold;})){
//        LOG_WARN(TAG, "Pogo Temperatures is NOT satisfy the condition");
        return false;
    }

    return true;
}


bool ChargerManager::checkTempConditionDischarging(const uint32_t& _cabId) {
    auto cab = m_cabManager->getCab(_cabId);
    if(!cab){
        return false;
    }
    std::vector<int32_t> pogoTemps {cab->m_data.m_pogoPinTemp[0], cab->m_data.m_pogoPinTemp[1],
                                    cab->m_data.m_pogoPinTemp[2], cab->m_data.m_pogoPinTemp[3]};

    if(cab->m_data.m_temp >= m_config.m_tempStopCharging){
        LOG_WARN(TAG, "Cabinet Temperatures is over threshold, Charger will be discharged");
        return true;
    }

    int32_t pogoTempThreshold = m_config.m_pogoTempStopCharging;
    if(std::any_of(pogoTemps.begin(), pogoTemps.end(), [pogoTempThreshold](int32_t x){return x >= pogoTempThreshold;})){
        LOG_WARN(TAG, "Cabinet :%d, Pogo Temperatures is over threshold: %d:%d:%d:%d, Charger will be discharged",
                 _cabId,
                 cab->m_data.m_pogoPinTemp[0],
                 cab->m_data.m_pogoPinTemp[1],
                 cab->m_data.m_pogoPinTemp[2],
                 cab->m_data.m_pogoPinTemp[3]);
        return true;
    }

    return false;
}

bool ChargerManager::checkChargerDischargingCondition(const std::shared_ptr<ChargerItem>& _charger){
    if(m_if){
        _charger->m_cur = m_if->getChargerCur(_charger->m_chargerId);
        _charger->m_vol = m_if->getChargerVol(_charger->m_chargerId);
    }else{
        auto bpData = m_bpManager->getData(_charger->m_cabCharging);

        _charger->m_cur = bpData.m_cur;
        _charger->m_vol = bpData.m_vol;
    }

    if(abs(_charger->m_cur) >= m_config.m_curChargingUpperThreshold ){
        auto cab = _charger->m_cabIds.find(_charger->m_cabCharging);
        cab->second->m_chargingCabState = CHARGING_CAB_BLACKLIST;
        cab->second->m_monitorTime.reset(CHARGER_CHECK_BACK_LIST_TIMEOUT);

        LOG_ERROR(TAG, "Charging Current is %d OVERLOAD, It's Dangerous. Charger will be discharged and put in the BLACKLIST", _charger->m_cur);
        return true;
    }

    if(abs(_charger->m_vol) >= m_config.m_volStopCharging && abs(_charger->m_cur) <= m_config.m_curChargingLowerThreshold){
        LOG_WARN(TAG, "BP is FULL, Voltage: %d, Current: %d, Charger will be discharged", _charger->m_vol, _charger->m_cur);
        return true;
    }

    if(abs(_charger->m_cur) >= m_config.m_curChargingLowerThreshold){
        _charger->m_currentMonitorTime.reset(CHARGER_CUR_MONITOR_TIME);
        _charger->m_retry = 0;
    }

    if(!_charger->m_currentMonitorTime.getRemainTime()){
        LOG_WARN(TAG, "Charger %d is no current during %d second, Please check again", _charger->m_chargerId, CHARGER_CUR_MONITOR_TIME/1000);
        _charger->m_retry++;
        if(_charger->m_retry >= CHARGER_RETRY_NUMBER){
            auto cab = _charger->m_cabIds.find(_charger->m_cabCharging);
            if(cab != _charger->m_cabIds.end()){
                LOG_WARN(TAG, "Cabinet %d is put in to BLACKLIST, because the current is too small. Maybe charger or BP are broken", _charger->m_cabCharging);
                cab->second->m_chargingCabState = CHARGING_CAB_BLACKLIST;
                cab->second->m_monitorTime.reset(CHARGER_CHECK_BACK_LIST_TIMEOUT);
            }
            _charger->m_retry = 0;
        }

        return true;
    }

    return false;
}

int32_t ChargerManager::checkConditionDischarge(const std::shared_ptr<ChargerItem> &_charger) {
    if(!m_cabManager->cabHaveBp(_charger->m_cabCharging)){
        LOG_WARN(TAG, "BP is removed from cabinet %d ", _charger->m_cabCharging);
        if(m_observer) {
            m_observer->onDischarged(_charger->m_chargerId, _charger->m_cabCharging, BP_IS_REMOVED);
        }
        startCmd(_charger, _charger->m_cabCharging, CHARGER_CMD_DISCHARGING);

        return 0;
    }

    if(checkTempConditionDischarging(_charger->m_cabCharging)){
        if(m_observer) {
            m_observer->onDischarged(_charger->m_chargerId, _charger->m_cabCharging, CAB_OVER_TEMP);
        }
        startCmd(_charger, _charger->m_cabCharging, CHARGER_CMD::CHARGER_CMD_DISCHARGING);
        return 0;
    }

    if(checkChargerDischargingCondition(_charger)){
        if(m_observer) {
            m_observer->onDischarged(_charger->m_chargerId, _charger->m_cabCharging, CHARGER_OVER_CUR);
        }
        startCmd(_charger, _charger->m_cabCharging, CHARGER_CMD::CHARGER_CMD_DISCHARGING);
        return 0;
    }

    return -1;
}

int32_t ChargerManager::checkCommonChargingCondition(const uint32_t& _cabId){
    if(!m_cabManager->cabHaveBp(_cabId)){
        return -1;
    }
    if(!checkTempConditionCharging(_cabId)){
        return -1;
    }
    if(!checkBpConditionCharging(_cabId)){
        return -1;
    }
    return 0;
}

int32_t ChargerManager::findCabToCharge(const std::shared_ptr<ChargerItem> &_charger) {
    if(_charger->m_cmd != CHARGER_CMD::CHARGER_CMD_NONE){
        return -1;
    }

    if(_charger->m_chargeForce.m_force){
        if(checkCommonChargingCondition(_charger->m_chargeForce.m_cabId) < 0){
            LOG_ERROR(TAG, "Force charge cabinet %d FAILURE, NOT enough charging condition", _charger->m_chargeForce.m_cabId);
            _charger->m_chargeForce.m_cabId = -1;
            _charger->m_chargeForce.m_force = false;
            return -1;
        }

        LOG_INFO(TAG, "Force charged cabinet %d, BP: %s", _charger->m_chargeForce.m_cabId,
                 m_bpManager->getData(_charger->m_chargeForce.m_cabId).m_sn.c_str());
        startCmd(_charger, _charger->m_chargeForce.m_cabId, CHARGER_CMD::CHARGER_CMD_CHARGING);
        return 0;
    }

    int32_t vol = 0;
    int32_t cab = -1;

    for(auto& cabId : _charger->m_cabIds){
        if(cabId.second->m_chargingCabState == CHARGING_CAB_STATE::CHARGING_CAB_DISABLE){
            continue;
        }

        if(cabId.second->m_chargingCabState == CHARGING_CAB_STATE::CHARGING_CAB_BLACKLIST){
            if(!cabId.second->m_monitorTime.getRemainTime()){
                cabId.second->m_chargingCabState = CHARGING_CAB_STATE::CHARGING_CAB_ENABLE;
                cabId.second->m_monitorTime.reset(CHARGER_CHECK_BACK_LIST_TIMEOUT);

                LOG_WARN(TAG, "Cabinet %d is put out to BLACKLIST and waiting for charge", cabId.first);
            }else{
                continue;
            }
        }

        if(cabId.first == _charger->m_dischargeForce.m_cabId && _charger->m_dischargeForce.m_force){
            continue;
        }

        if(checkCommonChargingCondition(cabId.first) < 0){
            continue;
        }

        /// Choose the BP have a Voltage that is smallest
        auto bpData = m_bpManager->getData(cabId.first);
        if(vol < bpData.m_vol){
            cab = cabId.first;
            vol = bpData.m_vol;
        }
    }

    if(cab >= 0 && vol){
        LOG_INFO(TAG, "Charger %d: Found cabinet %d that eligible for charging: BP %s have SOC: %d, VOL: %d",
                 _charger->m_chargerId,
                 cab,
                 m_bpManager->getData(cab).m_sn.c_str(),
                 m_bpManager->getData(cab).m_soc,
                 m_bpManager->getData(cab).m_vol);

        startCmd(_charger, cab, CHARGER_CMD::CHARGER_CMD_CHARGING);

        return 0;
    }

    return -1;
}