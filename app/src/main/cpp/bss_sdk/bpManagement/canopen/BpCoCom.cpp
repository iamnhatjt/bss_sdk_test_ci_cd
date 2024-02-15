//
// Created by vnbk on 02/06/2023.
//

#include "BpCoCom.h"
#include "Logger.h"

#include "app_co_init.h"
#include "od.h"

#define TAG "BpCoCom::BpCoCom"


void BpCoCom::bpManagementReceivedCallback(uint32_t _can_id, uint8_t* _data, void* _arg){
    auto bpManagement = (BpCoCom*)(_arg);
    switch(_can_id & 0xFFFFFF80){
        case CO_CAN_ID_TPDO_1:
        case CO_CAN_ID_TPDO_2:
        case CO_CAN_ID_TPDO_3:
        case CO_CAN_ID_TPDO_4:
        case CO_CAN_ID_RPDO_1:
        case CO_CAN_ID_RPDO_2:
            bpManagement->pdoProcess(_can_id, _data);
            break;
        default:
            break;
    }

    bpManagement->m_assignModule->recvAssignedData(_can_id, _data, 8);
}

std::shared_ptr<BpCoCom> BpCoCom::create(const std::shared_ptr<NodeIdControllerInterface>& _node_if,
                                         sm_co_if_t* _co_if,
                                         int32_t _bp_number,
                                         uint32_t _nodeIdOffset,
                                         bool _autoAssign){
    auto bpManager = std::shared_ptr<BpCoCom>(new BpCoCom(_co_if,
                                                          _bp_number,
                                                          _nodeIdOffset,
                                                          _autoAssign));
    if(bpManager->initialized(_node_if)){
        LOG_INFO(TAG, "Created Bp Manager SUCCESS");
        return bpManager;
    }
    return nullptr;
}

BpCoCom::BpCoCom(sm_co_if_t* _co_if,
                 int32_t _bp_number,
                 uint32_t _nodeIdOffset,
                 bool _autoAssign) : m_coInterface(_co_if),
                           m_bpAssignIndex(0),
                           m_autoAssign(_autoAssign),
                           m_currentCmd(nullptr),
                           m_nodeIdOffset(_nodeIdOffset),
                           m_bpNumber(_bp_number),
                           m_sync(true){
    m_coDevice = nullptr;
}

bool BpCoCom::initialized(const std::shared_ptr<NodeIdControllerInterface>& _nodeIdController) {
    if(!_nodeIdController || !m_coInterface){
        LOG_ERROR(TAG, "Parameters INVALID");
        return false;
    }

    m_assignModule = std::make_shared<BpAssignModule>(m_manager,
                                                      _nodeIdController,
                                                      m_nodeIdOffset);
    m_bpAssignIndex = 0;

    app_co_init(m_coInterface, bpManagementReceivedCallback, this);
    m_coDevice = &CO_DEVICE;

    resetCmd();

    LOG_DEBUG(TAG, "Initialized Bp Management SUCCESS");
    return true;
}

BpCoCom::~BpCoCom() noexcept {
    app_co_free();
    m_assignModule.reset();
}

void BpCoCom::pdoProcess(uint32_t _cobId, uint8_t *_data) {
    uint32_t cob_id = _cobId & 0xFFFFFF80;
    auto node_id = (uint8_t) (_cobId & 0x7F);
    uint8_t bp_id = node_id - m_nodeIdOffset;

    if(bp_id >= m_bpNumber)
        return;
    
    auto bp = m_manager->getBp(bp_id);
    
    switch(cob_id){
        case BP_VOL_CUR_TPDO_COBID:
            if(10*(uint32_t)CO_getUint16(_data) < 70000 && 10*(uint32_t)CO_getUint16(_data) > 40000){
                bp->m_info.m_vol = 10*(uint32_t)CO_getUint16(_data);
            }
            if(10*((int16_t)CO_getUint16(_data + 2)) > -20000
               && 10*((int16_t)CO_getUint16(_data + 2)) < 20000 ){
                bp->m_info.m_cur 		= 10*((int16_t)CO_getUint16(_data + 2));
            }
            
            bp->m_info.m_soc 	= _data[4];
            bp->m_info.m_state 	= _data[5];
            bp->m_info.m_status	= CO_getUint16(_data + 6);

            break;
        case BP_CELLS_VOL_1_TO_4:
            CO_memcpy((uint8_t*)(bp->m_info.m_cellVols), _data, 8);
            break;
        case BP_CELLS_VOL_5_TO_8:
            CO_memcpy((uint8_t*)(bp->m_info.m_cellVols + 4), _data, 8);
            break;
        case BP_CELLS_VOL_9_TO_12:
            CO_memcpy((uint8_t*)(bp->m_info.m_cellVols + 8), _data, 8);
            break;
        case BP_CELLS_VOL_13_TO_16:
            CO_memcpy((uint8_t*)(bp->m_info.m_cellVols + 12), _data, 8);
            break;
        case BP_TEMP_TPDO_COBID:
            CO_memcpy(bp->m_info.m_temps, _data, 6);
            for(auto& temp : bp->m_info.m_temps){
                if(temp > 110){
                    temp = 0;
                }
            }
            bp->m_chargingState = _data[6];
            break;
        case BP_SOH_CYCLE_COBID:
            bp->m_info.m_soh = _data[2];
            bp->m_info.m_cycle = CO_getUint16(_data);
            LOG_DEBUG(TAG, "Bp %d have SOH: %d, Cycle: %d", bp->m_id, bp->m_info.m_soh, bp->m_info.m_cycle);
            break;
        default:
            break;
    }
    bp->m_timeout.reset();
//    LOG_DEBUG(TAG, "Received data from BP %d", node_id);
}

int32_t BpCoCom::cmdReadSn(const std::shared_ptr<Bp>& _bp) {
    auto sdoClient = &m_coDevice->sdo_client;

    CO_Sub_Object_t subObject = {
            .p_data = _bp->m_sn,
            .attr = ODA_SDO_RW,
            .len = DEVICE_SN_LENGTH,
            .p_ext = nullptr,
            .p_temp_data = nullptr
    };

    CO_SDOclient_start_upload(sdoClient,
                              _bp->m_id + m_nodeIdOffset,
                              BMS_INDEX,
                              BMS_SERIAL_NUMBER_OBJECT_SUB_INDEX,
                              &subObject,
                              SDO_READ_OBJ_TIMEOUT_MS);
    return 0;
}

int32_t BpCoCom::cmdReadVersion(const std::shared_ptr<Bp>& _bp) {
    auto sdoClient = &m_coDevice->sdo_client;

    CO_Sub_Object_t subObject = {
            .p_data = _bp->m_version,
            .attr = ODA_SDO_RW,
            .len = DEVICE_VERSION_LENGTH,
            .p_ext = nullptr,
            .p_temp_data = nullptr
    };

    CO_SDOclient_start_upload(sdoClient,
                              _bp->m_id + m_nodeIdOffset,
                              BMS_VERSION_INDEX,
                              BMS_VERSION_SUB_INDEX,
                              &subObject,
                              SDO_READ_OBJ_TIMEOUT_MS);
    return 0;
}

int32_t BpCoCom::cmdReadDeviceAssigned(const std::shared_ptr<Bp>& _bp) {
    auto sdoClient = &m_coDevice->sdo_client;

    CO_Sub_Object_t subObject = {
            .p_data = _bp->m_assignedSn,
            .attr = ODA_SDO_RW,
            .len = DEVICE_SN_LENGTH,
            .p_ext = nullptr,
            .p_temp_data = nullptr
    };

    LOG_DEBUG(TAG, "Start read the device that is assigned to BP");

    CO_SDOclient_start_upload(sdoClient,
                              _bp->m_id + m_nodeIdOffset,
                              BMS_VEHICLE_SN_INDEX,
                              BMS_MATTED_DEV_SUB_INDEX,
                              &subObject,
                              SDO_READ_OBJ_TIMEOUT_MS);

    return 0;
}

int32_t BpCoCom::cmdWriteDeviceAssigned(const std::shared_ptr<Bp>& _bp, const std::string& _devSn) {
    auto sdoClient = &m_coDevice->sdo_client;

    strcpy((char*)_bp->m_assignedSn, _devSn.data());
    _bp->m_assignedSn[_devSn.length()] = '\0';

    CO_Sub_Object_t subObject = {
            .p_data = _bp->m_assignedSn,
            .attr = ODA_SDO_RW,
            .len = DEVICE_SN_LENGTH,
            .p_ext = nullptr,
            .p_temp_data = nullptr
    };

    LOG_DEBUG(TAG, "Start Write device sn %s to BP %d", _devSn.c_str(), _bp->m_id);

    CO_SDOclient_start_download(sdoClient,
                                _bp->m_id + m_nodeIdOffset,
                                BMS_VEHICLE_SN_INDEX,
                                BMS_MATTED_DEV_SUB_INDEX,
                                &subObject,
                                SDO_WRITE_OBJ_TIMEOUT_MS);

    return 0;
}

int32_t BpCoCom::cmdReboot(const std::shared_ptr<Bp>& _bp) {
    auto sdoClient = &m_coDevice->sdo_client;

    _bp->m_rebootState = 1;

    CO_Sub_Object_t subObject = {
            .p_data = &_bp->m_rebootState,
            .attr = ODA_SDO_RW,
            .len = 1,
            .p_ext = nullptr,
            .p_temp_data = nullptr
    };

    LOG_WARN(TAG, "Start reboot BP, node ID: %d", _bp->m_id + m_nodeIdOffset);

    CO_SDOclient_start_download(sdoClient,
                                _bp->m_id + m_nodeIdOffset,
                                SM_DEVICE_BOOT_INDEX,
                                SN_DEVICE_REBOOT_SUB_INDEX,
                                &subObject,
                                SDO_WRITE_OBJ_TIMEOUT_MS);

    return 0;
}

int32_t BpCoCom::cmdCharging(const std::shared_ptr<Bp>& _bp) {
    auto sdoClient = &m_coDevice->sdo_client;

    _bp->m_chargingState = BMS_STATE_CHARGING;

    CO_Sub_Object_t subObject = {
            .p_data = &_bp->m_chargingState,
            .attr = ODA_SDO_RW,
            .len = 1,
            .p_ext = nullptr,
            .p_temp_data = nullptr
    };

    CO_SDOclient_start_download(sdoClient,
                                _bp->m_id + m_nodeIdOffset,
                                BMS_INDEX,
                                BMS_MAIN_SWITCH_SUB_INDEX,
                                &subObject,
                                SDO_WRITE_OBJ_TIMEOUT_MS);

    return 0;
}

int32_t BpCoCom::cmdDischarging(const std::shared_ptr<Bp>& _bp) {
    auto sdoClient = &m_coDevice->sdo_client;

    _bp->m_chargingState = BMS_STATE_DISCHARGING;

    CO_Sub_Object_t subObject = {
            .p_data = &_bp->m_chargingState,
            .attr = ODA_SDO_RW,
            .len = 1,
            .p_ext = nullptr,
            .p_temp_data = nullptr
    };

    CO_SDOclient_start_download(sdoClient,
                                _bp->m_id + m_nodeIdOffset,
                                BMS_INDEX,
                                BMS_MAIN_SWITCH_SUB_INDEX,
                                &subObject,
                                SDO_WRITE_OBJ_TIMEOUT_MS);

    return 0;
}

int32_t BpCoCom::cmdHandle(const std::shared_ptr<Bp::BpCmd>& _cmd) {
    if(_cmd->m_cmd >= BP_COMMAND::BP_CMD_NUMBER){
        return -1;
    }
    auto bp = _cmd->m_bp;
    std::string value;
    if(bp->m_assignStatus != BP_ASSIGN_STATE::BP_ASSIGN_FAILURE){
        switch(_cmd->m_cmd){
            case BP_COMMAND::BP_CMD_REBOOT:
                cmdReboot(bp);
                break;
            case BP_COMMAND::BP_CMD_CHARGED:
                cmdCharging(bp);
                break;
            case BP_COMMAND::BP_CMD_DISCHARGED:
                cmdDischarging(bp);
                break;
            case BP_COMMAND::BP_CMD_READ_SN:
                cmdReadSn(bp);
                break;
            case BP_COMMAND::BP_CMD_READ_VERSION:
                cmdReadVersion(bp);
                break;
            case BP_COMMAND::BP_CMD_READ_ASSIGNED_DEVICE:
                cmdReadDeviceAssigned(bp);
                break;
            case BP_COMMAND::BP_CMD_WRITE_ASSIGNED_DEVICE:
                value = (char*)_cmd->m_data;
                cmdWriteDeviceAssigned(bp, value);
                break;
            default:
                break;
        }
    }
    return 0;
}

int32_t BpCoCom::cmdHandleResponse(){
    CO_SDO_return_t status = CO_SDO_get_status(&m_coDevice->sdo_client);
    if(status == CO_SDO_RT_abort){

        LOG_ERROR(TAG, "Bp %d command FAILURE: %s, TX_ERROR: 0x%02X, RX_ERROR: 0x%02X",
                  m_currentCmd->m_bp->m_id,
                  convertBpCmdToString(m_currentCmd->m_cmd).c_str(),
                  m_coDevice->sdo_client.tx_abort_code,
                  m_coDevice->sdo_client.rx_abort_code);

        if(m_currentCmd->m_callback){
            m_currentCmd->m_callback(m_currentCmd->m_cmd,
                                     BP_CMD_FAILURE,
                                     m_currentCmd->m_bp->m_id,
                                     m_currentCmd->m_data,
                                     m_currentCmd->m_arg);
            m_currentCmd->m_bp->m_timeout.reset();
        }

        resetCmd();
        m_manager->resetCurrentCmd();
    }else if(status == CO_SDO_RT_success){
        LOG_DEBUG(TAG, "Bp %d command SUCCESS: %s", m_currentCmd->m_bp->m_id, convertBpCmdToString(m_currentCmd->m_cmd).c_str());
        if(m_currentCmd->m_callback){
            m_currentCmd->m_callback(m_currentCmd->m_cmd,
                                     BP_CMD_SUCCESS,
                                     m_currentCmd->m_bp->m_id,
                                     m_currentCmd->m_data,
                                     m_currentCmd->m_arg);

            m_currentCmd->m_bp->m_timeout.reset();
        }

        resetCmd();
        m_manager->resetCurrentCmd();
    }
    return 0;
}

void BpCoCom::addManager(std::shared_ptr<BpManagerInterface> _manager) {
    m_manager = _manager;
    m_assignModule->addManager(_manager);
}

int32_t BpCoCom::assign(const std::shared_ptr<Bp>& _bp) {
    if(_bp->m_assignStatus == BP_ASSIGN_STATE::BP_ASSIGNED){
        m_manager->notifyAssigningState(_bp->m_id,
                                          BP_ASSIGN_STATE::BP_ASSIGNED,
                                        _bp->m_info.m_sn,
                                        _bp->m_info.m_version);
    }
    m_assignModule->addBp(_bp);
    return 0;
}

bool BpCoCom::sync(bool _enable) {
    if(_enable == m_sync){
        return m_sync;
    }

    if(!_enable){
        CO_disable_sync_mode(&CO_DEVICE);
        m_sync = false;
    }else{
        CO_recover_sync_mode(&CO_DEVICE, &od_temp_comm_profile_para);
        m_sync = true;
    }
    return m_sync;
}

int32_t BpCoCom::setCmd(const std::shared_ptr<Bp::BpCmd>& _cmd) {
    m_currentCmd = _cmd;
    return cmdHandle(_cmd);
}

int32_t BpCoCom::resetCmd() {
    m_currentCmd.reset();
    CO_SDO_reset_status(&m_coDevice->sdo_client);
    return 0;
}

void BpCoCom::getAssignBp(){
    if(m_assignModule->getAssignState() == BP_ASSIGN_STATE::BP_ASSIGN_FAILURE){
        uint32_t index = m_bpAssignIndex;
        for(; index < m_bpNumber; index++){
            auto bp = m_manager->getBp(index);
            if(bp->m_assignStatus == BP_ASSIGN_STATE::BP_ASSIGN_FAILURE){
                LOG_DEBUG(TAG, "New BP index %d is started assigning process", index);
                m_assignModule->addBp(bp);

                m_bpAssignIndex += 1;
                return;
            }
        }
        if(index >= m_bpNumber){
            m_bpAssignIndex = 0;
        }
    }
}

bool BpCoCom::isBusy(){
    return CO_SDO_get_status(&m_coDevice->sdo_client) != CO_SDO_RT_idle;
}

int32_t BpCoCom::process() {
    app_co_process();

    if(m_sync && m_assignModule->process() < 0){
        if(m_autoAssign){
            getAssignBp();
        }
    }

    if(m_sync) {
        if (m_currentCmd) {
            cmdHandleResponse();
        } else {
            CO_SDO_return_t status = CO_SDO_get_status(&m_coDevice->sdo_client);
            if (status == CO_SDO_RT_abort) {
                LOG_ERROR(TAG, "CanOpen SDO Client returned ABORT by itself");
                CO_SDO_reset_status(&m_coDevice->sdo_client);
            }
        }
    }
    return 0;
}