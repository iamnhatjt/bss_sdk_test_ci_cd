//
// Created by vnbk on 01/06/2023.
//

#include "CabManagement.h"

#include <utility>

#include "utils.h"
#include "Logger.h"

#define TAG "CabinetManager"

std::shared_ptr<CabinetManager> CabinetManager::create(std::shared_ptr<CabComInterface> _if,
                                                       std::shared_ptr<BpManagerInterface> _bp_service,
                                                       uint32_t _number,
                                                       int32_t _cabTempOffset,
                                                       int32_t _pogoTempOffset){
    auto manager = std::shared_ptr<CabinetManager>(new CabinetManager(std::move(_if),
                                                                      std::move(_bp_service),
                                                                      _number,
                                                                      _cabTempOffset,
                                                                      _pogoTempOffset));
    if(manager->initialized()){
        LOG_DEBUG(TAG, "Created CabinetManager SUCCESS");
        return manager;
    }
    LOG_ERROR(TAG, "Created CabinetManager FAILURE");
    return nullptr;
}

CabinetManager::CabinetManager(std::shared_ptr<CabComInterface> _if,
                               std::shared_ptr<BpManagerInterface> _bp_service,
                               uint32_t _number,
                               int32_t _cabTempOffset,
                               int32_t _pogoTempOffset) : CabManagerInterface(std::move(_if)),
                                                             m_bpManager(std::move(_bp_service)),
                                                             m_cabNumber(_number),
                                                             m_currentCab(0),
                                                             m_bpAssignIndex(0),
                                                             m_forceAssigning(false),
                                                             m_cabAssigning(nullptr),
                                                             m_assigningCount(0),
                                                             m_cabTempOffset(_cabTempOffset),
                                                             m_pogoTempOffset(_pogoTempOffset){
    m_waitBpAssigning = false;
}

bool CabinetManager::initialized() {
    if(m_cabNumber <= 0 || !m_bpManager){
        return false;
    }

    for(int i = 0; i < m_cabNumber; i++){
        auto cab = std::make_shared<Cab>(i);
        m_cabs.push_back(cab);
    }
    m_currentCab = 0;
    return true;
}

CabinetManager::~CabinetManager(){
    m_cabs.clear();
}

bool CabinetManager::isConnected(const uint32_t& _id){
    return m_cabs[_id]->m_data.m_isConnected;
}

bool CabinetManager::setUpgradingState(const uint32_t &_id, bool _isUpgrading) {
    m_cabs[_id]->m_data.m_isUpgrading = _isUpgrading;
    return _isUpgrading;
}

std::vector<uint32_t> CabinetManager::getEmptyCabs(){
    std::vector<uint32_t> list;
    LOG_TRACE(TAG, "Get cabinet list that do NOT have battery pack");
    for(auto& item : m_cabs){
        if(item->m_data.m_bpSn.empty()){
            list.push_back(item->m_id);
        }
    }
    return list;
}

std::vector<uint32_t> CabinetManager::getCabsWithBp(){
    std::vector<uint32_t> list;
    LOG_TRACE(TAG, "Get cabinet list that have battery pack");
    for(auto& item : m_cabs){
        if(!item->m_data.m_bpSn.empty() && item->m_data.m_bpSoc != 0){
            list.push_back(item->m_id);
        }
    }
    return list;
}

bool CabinetManager::cabHaveBp(const uint32_t &_id) {
    if(_id >= m_cabNumber){
        return false;
    }
    return m_cabs[_id]->m_data.m_isConnected && !m_cabs[_id]->m_data.m_bpSn.empty() && m_cabs[_id]->m_data.m_bpSoc;
}

const std::vector<std::shared_ptr<Cab>>& CabinetManager::getCabs(){
    LOG_TRACE(TAG, "Get cabinet list");
    return m_cabs;
}

std::shared_ptr<Cab>& CabinetManager::getCab(const uint32_t& _id){
    std::lock_guard<std::mutex> lockGuard(m_lock);
    return m_cabs[_id];
}

int32_t CabinetManager::openDoor(const uint32_t& _id){
    LOG_DEBUG(TAG, "Open Cabinet Door command");
    int32_t ret = m_interface->openDoor(_id);
    if(ret < 0){
        LOG_ERROR(TAG, "Control Cabinet Door FAILURE");
        return ret;
    }
    return ret;
}

int32_t CabinetManager::ctlFan(const uint32_t& _id, uint8_t _value) {
    LOG_DEBUG(TAG, "Control Cabinet Fan command");
    int32_t ret = m_interface->ctlFan(_id, _value);
    if(ret < 0){
        LOG_ERROR(TAG, "Control Cabinet Fan FAILURE");
        return ret;
    }
    if(m_observer){
        m_observer->onCabChangedFanStatus(_id, _value);
    }
    return ret;
}

int32_t CabinetManager::ctlChargerSwitch(const uint32_t& _id, uint8_t _value) {
    LOG_DEBUG(TAG, "Control Charger switch cabinet command: %s", _value ? "CHARGED" : "DISCHARGED");
    int32_t ret = m_interface->ctlCharger(_id, _value);
    if(ret < 0){
        LOG_ERROR(TAG, "Control Cabinet ChargerCom FAILURE");
        return ret;
    }
    if(m_observer){
        m_observer->onCabChangedChargerStatus(_id, _value);
    }
    return ret;
}

int32_t CabinetManager::ctlNodeId(const uint32_t &_id, uint8_t _value) {
    LOG_DEBUG(TAG, "Control NodeID command");
    return 0;
   /* int32_t ret = m_interface->setNodeId(_id, (NodeIdControllerInterface::CANBUS_NODE_ID)_value);
    if(ret < 0){
        LOG_ERROR(TAG, "Control NodeID FAILURE");
        return ret;
    }
    return ret;*/
}

int32_t CabinetManager::assignBp(const uint32_t& _id, int32_t _count){
    std::lock_guard<std::mutex> lockGuard(m_lock);
    if(m_bpManager->getAssigningStatus(_id) == (int32_t)BP_ASSIGN_STATE::BP_ASSIGNED){
        LOG_INFO(TAG, "Bp is assigned in the Cabinet %d", _id);
        return 0;
    }

    if(!checkAssignCondition(_id)){
        LOG_ERROR(TAG, "BP is NO assigning condition");
        return -1;
    }

    if(!_count || _count >= 10){
        return m_bpManager->assign(_id);
    }
    m_assigningCount = _count - 1;
    m_cabAssigning = m_cabs[_id];
    m_forceAssigning = true;

    m_cabAssigning->m_retry = 0;
    m_cabAssigning->m_reconnectTime.reset();

    return m_bpManager->assign(_id);;
}

int32_t CabinetManager::assignDeviceToBp(const uint32_t& _id, const std::string &_dev_sn) {
    if(m_bpManager->getAssigningStatus(_id) != (int32_t)BP_ASSIGN_STATE::BP_ASSIGNED){
        LOG_ERROR(TAG, "BP is in the cabinet %d that is NOT Assigned", _id);
        if(m_observer) {
            m_observer->onDeviceIsAssignedToBp(_id, BSS_RET_FAILURE, _dev_sn);
        }
        return -1;
    }

    return m_bpManager->assignDeviceToBp(_id, _dev_sn);
}

void CabinetManager::updateData(BssCabData &_data, CabComInterface::CabComData &_comData) {
    if(_comData.m_fanStt == CAB_FAN_TURN_OFF_STATUS || _comData.m_fanStt == CAB_FAN_TURN_ON_STATUS){
        _data.m_fanStt = _comData.m_fanStt;
    }
    if(_comData.m_chargerStt == CAB_CHARGER_TURN_OFF_STATUS || _comData.m_chargerStt == CAB_CHARGER_TURN_ON_STATUS){
        _data.m_chargerSwitchStt = _comData.m_chargerStt;
    }
    if(_comData.m_doorStt == CAB_DOOR_OPEN_STATUS || _comData.m_doorStt == CAB_DOOR_CLOSE_STATUS){
        _data.m_doorStt = _comData.m_doorStt;
    }

    if(_comData.m_temp < CAB_TEMP_LOWER_THRESHOLD){
        _data.m_temp = CAB_TEMP_LOWER_THRESHOLD;
    }else if(_comData.m_temp > CAB_TEMP_UPPER_THRESHOLD){
        _data.m_temp = CAB_TEMP_UPPER_THRESHOLD;
    }else{
        _data.m_temp = _comData.m_temp + m_cabTempOffset;
    }

    for(int index = 0; index < 4; index++){
        if(_comData.m_pogoTemps[index] < CAB_POGO_TEMP_LOWER_THRESHOLD){
            _data.m_pogoPinTemp[index] = CAB_POGO_TEMP_LOWER_THRESHOLD;
        }else if(_comData.m_pogoTemps[index] > CAB_POGO_TEMP_UPPER_THRESHOLD){
            _data.m_pogoPinTemp[index] = CAB_POGO_TEMP_UPPER_THRESHOLD;
        }else{
            _data.m_pogoPinTemp[index] = _comData.m_pogoTemps[index] + m_pogoTempOffset;
        }
    }
}

bool CabinetManager::checkAssignCondition(const uint32_t &_id) {
    return m_cabs[_id]->m_data.m_isConnected &&
           (m_cabs[_id]->m_data.m_doorStt == CAB_DOOR_CLOSE_STATUS) &&
           !m_cabs[_id]->m_data.m_isUpgrading &&
           (m_bpManager->getAssigningStatus(_id) == (int32_t) BP_ASSIGN_STATE::BP_ASSIGN_FAILURE); /*&&
           (m_bpManager->getAssigningRetriedNumber(_id) <= BP_ASSIGNING_RETRIED_MAX);*/
}

void CabinetManager::resetForceCabAssigning(){
    m_assigningCount = 0;
    m_forceAssigning = false;
    m_cabAssigning = nullptr;
}

void CabinetManager::getAssignBp() {
    if(m_forceAssigning && m_cabAssigning && m_assigningCount){
        std::lock_guard<std::mutex> lockGuard(m_lock);

        if(m_bpManager->getAssigningStatus(m_cabAssigning->m_id) == (int32_t) BP_ASSIGN_STATE::BP_ASSIGNED){
            resetForceCabAssigning();
            return;
        }

        m_bpManager->assign(m_cabAssigning->m_id);
        m_assigningCount--;
        if(!m_assigningCount){
            resetForceCabAssigning();
        }
        m_waitBpAssigning = true;
        return;
    }

    int index = m_bpAssignIndex;
    for (; index < m_cabNumber; index++) {
        m_bpAssignIndex++;
        if(checkAssignCondition(index)){
            LOG_TRACE(TAG, "New BP index %d is started assigning process", index);
            m_bpManager->assign(index);

            std::lock_guard<std::mutex> lockGuard(m_lock);
            m_waitBpAssigning = true;
            break;
        }
    }
    if (m_bpAssignIndex >= m_cabNumber) {
        m_bpAssignIndex = 0;
    }
}

void CabinetManager::process() {
    if(!m_waitBpAssigning){
        getAssignBp();
    }

    auto cab = m_cabs[m_currentCab];
    if(!cab->m_data.m_isConnected && cab->m_retry >= CAB_RECONNECT_RETRY_COUNT){
        if(!cab->m_reconnectTime.getRemainTime() /*&& !m_waitBpAssigning*/){
            cab->m_retry = 0;
            cab->m_reconnectTime.reset();
            LOG_WARN(TAG, "Check cabinet %d connection again", cab->m_id);
        }
    }else{
        CabComInterface::CabComData cabComData;
        int32_t ret = m_interface->getAll(cab->m_id, cabComData);
        if (ret >= 0) {
            updateData(cab->m_data, cabComData);
            if (cab->m_prevDoorStatus != cabComData.m_doorStt) {
                LOG_WARN(TAG, "Cabinet Door Status is changed");
                if (m_observer){
                    m_observer->onCabChangedDoorStatus(cab->m_id, cabComData.m_doorStt); /// Reverse Door Status on the Slave
                }

                cab->m_prevDoorStatus = cabComData.m_doorStt;

                m_bpManager->resetBp(cab->m_id);
                if(cab->m_data.m_doorStt == CAB_DOOR_CLOSE_STATUS){
                    LOG_INFO(TAG,"Cabinet Door %d is closed, Force Assign BP", cab->m_id);
                    assignBp(cab->m_id, 2);
                }
            }

            if(!cab->m_data.m_isConnected && m_observer) {
                m_observer->onCabConnectionChanged(cab->m_id, CAB_STATE_CONNECTED);
            }
            cab->m_data.m_isConnected = true;
            cab->m_retry = 0;
            cab->m_connectionAvailable = true;

            auto bpData = m_bpManager->getData(cab->m_id);
            if(!cab->m_data.m_bpSn.empty()){
                if(!cab->m_data.m_bpSoc) {
                    if (m_observer && bpData.m_soc) {
                        LOG_INFO(TAG, "Now the new PIN actually connects to Cabinet: %s, SOC: %d", bpData.m_sn.c_str(), bpData.m_soc);
                        cab->m_data.m_bpSoc = bpData.m_soc;
                        m_observer->onBpConnected(cab->m_id, bpData.m_sn, bpData.m_version);
                    }
                }
                cab->m_data.m_bpSoc = bpData.m_soc;

                if(!cab->m_data.m_isCharged){
                    if(cab->m_data.m_chargerSwitchStt == !CAB_CHARGER_TURN_ON_STATUS && bpData.m_state == BP_STATE_CHARGING) {
                        cab->m_data.m_isCharged = true;
                        LOG_INFO(TAG, "Cabinet %d is charged", cab->m_id);
                    }
                }
            }

            if(cab->m_data.m_isCharged){
                if(cab->m_data.m_chargerSwitchStt == !CAB_CHARGER_TURN_OFF_STATUS || bpData.m_state != BP_STATE_CHARGING){
                    cab->m_data.m_isCharged = false;
                    LOG_WARN(TAG, "Cabinet %d is discharged", cab->m_id);
                }
            }

        }else{
            cab->m_retry++;
            if(cab->m_retry >= CAB_RECONNECT_RETRY_COUNT){
                if(cab->m_connectionAvailable){
                    LOG_ERROR(TAG, "Total retry count Cabinet %d : %d", cab->m_id, cab->m_retry);
                    cab->m_retry = 0;
                }
                cab->m_data.m_isConnected = false;
                cab->m_data.m_bpSn.clear();
                cab->m_data.m_bpSoc = 0;

                int timeout = getRandom(CAB_RECONNECT_TIME_LOWER, CAB_RECONNECT_TIME_UPPER);
                LOG_TRACE(TAG, "Cab reconnect timeout = %d", timeout);

                cab->m_reconnectTime.reset(timeout);
                m_bpManager->resetBp(cab->m_id);

                LOG_WARN(TAG, "Cabinet %d is disconnected. Try again after in a few minutes", cab->m_id);

                if(m_observer){
                    m_observer->onCabConnectionChanged(cab->m_id, CAB_STATE_DISCONNECTED);
                }
            }
        }
    }

    m_currentCab++;
    if(m_currentCab >= m_cabs.size())
        m_currentCab = 0;
}

void CabinetManager::onBpConnected(const uint32_t &_id,
                                   const std::string &_sn,
                                   const std::string &_version) {
    m_lock.lock();
    m_cabs[_id]->m_data.m_bpSn = _sn;
    m_cabs[_id]->m_data.m_bpSoc = m_bpManager->getData(_id).m_soc;
    m_waitBpAssigning = false;

    if (m_forceAssigning) {
        resetForceCabAssigning();
    }
    m_lock.unlock();

    if (m_observer && m_cabs[_id]->m_data.m_bpSoc != 0) {
        LOG_INFO(TAG, "Cabinet have BP connected: %s, SOC: %d", _sn.c_str(), m_cabs[_id]->m_data.m_bpSoc);
        m_observer->onBpConnected(_id, _sn, _version);
    }
}

void CabinetManager::onBpDisconnected(const uint32_t &_id, const std::string &_sn) {
    m_lock.lock();

    m_waitBpAssigning = false;
    m_cabs[_id]->m_data.m_bpSn.clear();
    m_cabs[_id]->m_data.m_bpSoc = 0;

    m_lock.unlock();

    if (m_observer) {
        m_observer->onBpDisconnected(_id, _sn);
    }
}

void CabinetManager::onBpAssignStatus(const uint32_t &_id, int32_t _status) {
    auto assignState = (BP_ASSIGN_STATE)_status;
    if(assignState == BP_ASSIGN_STATE::BP_ASSIGN_FAILURE || assignState == BP_ASSIGN_STATE::BP_ASSIGNED){

        m_lock.lock();
        m_waitBpAssigning = false;

        if(m_forceAssigning && m_assigningCount){
            m_assigningCount--;
        }
        m_lock.unlock();
    }
    if(m_observer){
        if(assignState == BP_ASSIGN_STATE::BP_ASSIGNED){
            m_observer->onCabTryReadBp(_id, BSS_RET_SUCCESS);
        }else if(assignState == BP_ASSIGN_STATE::BP_ASSIGN_FAILURE){
            m_observer->onCabTryReadBp(_id, BSS_RET_FAILURE);
        }
    }
}

void CabinetManager::onDeviceIsAssigned(const uint32_t &_id,
                                        int32_t _status,
                                        const std::string &_bp_sn,
                                        const std::string &_dev_type,
                                        const std::string &_dev_sn) {
    if(m_observer){
        m_observer->onDeviceIsAssignedToBp(_id,
                                           _status,
                                           _dev_sn);
    }
}
