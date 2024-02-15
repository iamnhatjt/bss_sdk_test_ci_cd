//
// Created by vnbk on 05/09/2023.
//

#include "BssModbusCom.h"

#include <utility>
#include <algorithm>
#include "Logger.h"

#include "BssModbusDefine.h"

#define TAG "BssModbusCom"

std::shared_ptr<BssModbusCom> BssModbusCom::create(std::shared_ptr<ModbusMasterInterface>& _modbusMaster,
                                                   std::shared_ptr<BpManagerInterface> _bpManager,
                                                   int32_t _baudRate) {
    auto com = std::shared_ptr<BssModbusCom>(new BssModbusCom(_modbusMaster, std::move(_bpManager), _baudRate));
    if(com->initialized()){
        LOG_INFO(TAG, "Created Bss Modbus Communication SUCCESS");
        return com;
    }
    return nullptr;
}

BssModbusCom::BssModbusCom(std::shared_ptr<ModbusMasterInterface> &_modbusMaster,
                           std::shared_ptr<BpManagerInterface> _bpManager,
                           int32_t _baudRate) : m_modbusMaster(_modbusMaster),
                                                m_bpManager(std::move(_bpManager)),
                                                m_baudRate(_baudRate) {
    m_idOffset = SLAVE_ID_OFFSET;
}

bool BssModbusCom::initialized() {
    if(!m_modbusMaster){
        LOG_ERROR(TAG, "Parameters INVALID");
        return false;
    }
    if(m_modbusMaster->open(m_baudRate)){
        LOG_ERROR(TAG, "Could Not Open Modbus RTU Port for BSS Communication");
        return false;
    }

    return true;
}

int32_t BssModbusCom::openDoor(uint32_t _id) {
    std::lock_guard<std::mutex> lockGuard(m_lock);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int32_t ret = m_modbusMaster->writeSingleRegister(_id + m_idOffset, MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_CAB_OPEN_DOOR), BSS_MODBUS_OPEN_DOOR);
    if(ret != MODBUS_ERROR_NONE){
        LOG_ERROR(TAG, "Open Cabinet %d FAILURE", _id);
        return -1;
    }
    return 0;
}

int32_t BssModbusCom::getDoorStatus(uint32_t _id) {
    std::lock_guard<std::mutex> lockGuard(m_lock);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    uint16_t value;
    int32_t ret = m_modbusMaster->readInputRegisters(_id + m_idOffset, MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_CAB_STATUS_TEMP), 1, &value);

    if(ret != MODBUS_ERROR_NONE){
        LOG_ERROR(TAG,"Could NOT read Door status");
        return -1;
    }
    uint8_t status = MODBUS_RTU_REG_GET_HIGH_BYTE(value);

    return (status >> BSS_MODBUS_CAB_DOOR_STT_INDEX) & 0x01;
}

int32_t BssModbusCom::ctlFan(uint32_t _id, uint32_t _value) {
    std::lock_guard<std::mutex> lockGuard(m_lock);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int32_t ret = m_modbusMaster->writeSingleRegister(_id + m_idOffset, MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_CAB_CTL_FAN), _value);
    if(ret != MODBUS_ERROR_NONE){
        LOG_ERROR(TAG, "Control FAN %d FAILURE", _id);
        return -1;
    }
    return 0;
}

int32_t BssModbusCom::getFanStatus(uint32_t _id) {
    std::lock_guard<std::mutex> lockGuard(m_lock);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    uint16_t value;
    int32_t ret = m_modbusMaster->readInputRegisters(_id + m_idOffset, MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_CAB_STATUS_TEMP), 1, &value);

    if(ret != MODBUS_ERROR_NONE){
        LOG_ERROR(TAG,"Could NOT read FAN status");
        return -1;
    }
    uint8_t status = MODBUS_RTU_REG_GET_HIGH_BYTE(value);

    return (status >> BSS_MODBUS_CAB_FAN_STT_INDEX) & 0x01;
}

int32_t BssModbusCom::ctlCharger(uint32_t _id, uint32_t _value) {
    std::lock_guard<std::mutex> lockGuard(m_lock);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    int32_t ret = m_modbusMaster->writeSingleRegister(_id + m_idOffset, MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_CAB_CTL_CHARGER_SWITCH), _value);
    if(ret != MODBUS_ERROR_NONE){
        LOG_ERROR(TAG, "Control Charger switch %d FAILURE", _id);
        return -1;
    }
    return 0;
}

int32_t BssModbusCom::getChargerStatus(uint32_t _id) {
    std::lock_guard<std::mutex> lockGuard(m_lock);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    uint16_t value;
    int32_t ret = m_modbusMaster->readInputRegisters(_id + m_idOffset, MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_CAB_STATUS_TEMP), 1, &value);

    if(ret != MODBUS_ERROR_NONE){
        LOG_ERROR(TAG,"Could NOT read Charger status");
        return -1;
    }
    uint8_t status = MODBUS_RTU_REG_GET_HIGH_BYTE(value);

    return (status >> BSS_MODBUS_CAB_CHARGER_SWITCH_STT_INDEX) & 0x01;
}

int32_t BssModbusCom::getTemp(uint32_t _id) {
    std::lock_guard<std::mutex> lockGuard(m_lock);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    uint16_t value;
    int32_t ret = m_modbusMaster->readInputRegisters(_id + m_idOffset, MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_CAB_STATUS_TEMP), 1, &value);

    if(ret != MODBUS_ERROR_NONE){
        LOG_ERROR(TAG,"Could NOT read Temperature status");
        return -1;
    }

    return MODBUS_RTU_REG_GET_LOW_BYTE(value);
}

std::vector<int32_t> BssModbusCom::getPogoPinTemp(uint32_t _id) {
    std::lock_guard<std::mutex> lockGuard(m_lock);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::vector<int32_t> pogoTemps;
    uint16_t value[2];
    int32_t ret = m_modbusMaster->readInputRegisters(_id + m_idOffset, MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_CAB_POGO_TEMP_12), 2, &value[0]);

    if(ret != MODBUS_ERROR_NONE){
        LOG_ERROR(TAG,"Could NOT read Temperature status");
    }else{
        pogoTemps.push_back(MODBUS_RTU_REG_GET_LOW_BYTE(value[0]));
        pogoTemps.push_back(MODBUS_RTU_REG_GET_HIGH_BYTE(value[0]));
        pogoTemps.push_back(MODBUS_RTU_REG_GET_LOW_BYTE(value[1]));
        pogoTemps.push_back(MODBUS_RTU_REG_GET_HIGH_BYTE(value[2]));
    }

    return pogoTemps;
}

int32_t BssModbusCom::getAll(uint32_t _id, CabComData& _info) {
    std::lock_guard<std::mutex> lockGuard(m_lock);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int32_t ret = m_modbusMaster->readInputRegisters(_id + m_idOffset,
                                                     MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_CAB_STATUS_TEMP),
                                                     BSS_MODBUS_SYNC_REG_NUMBER,
                                                     &m_inputRegister[0]);

    if(ret != MODBUS_ERROR_NONE){
//        LOG_ERROR(TAG,"Could NOT read all status at cabinet: %d", _id);
        return -1;
    }

    _info.m_id = (int32_t)_id;
    _info.m_fanStt = (MODBUS_RTU_REG_GET_LOW_BYTE(m_inputRegister[BSS_MODBUS_CAB_STATUS_TEMP_REG_INDEX]) >> BSS_MODBUS_CAB_FAN_STT_INDEX) & 0x01;
    _info.m_doorStt = (MODBUS_RTU_REG_GET_LOW_BYTE(m_inputRegister[BSS_MODBUS_CAB_STATUS_TEMP_REG_INDEX]) >> BSS_MODBUS_CAB_DOOR_STT_INDEX) & 0x01;
    _info.m_chargerStt = (MODBUS_RTU_REG_GET_LOW_BYTE(m_inputRegister[BSS_MODBUS_CAB_STATUS_TEMP_REG_INDEX]) >> BSS_MODBUS_CAB_CHARGER_SWITCH_STT_INDEX) & 0x01;
    _info.m_temp = MODBUS_RTU_REG_GET_HIGH_BYTE(m_inputRegister[BSS_MODBUS_CAB_STATUS_TEMP_REG_INDEX]);

    LOG_TRACE(TAG, "Cabinet %d: FAN:%d, Door:%d, Charger: %d", _id, _info.m_fanStt, _info.m_doorStt, _info.m_chargerStt);

    _info.m_pogoTemps[0] = MODBUS_RTU_REG_GET_LOW_BYTE(m_inputRegister[BSS_MODBUS_CAB_POGO_TEMP_12_REG_INDEX]);
    _info.m_pogoTemps[1] = MODBUS_RTU_REG_GET_HIGH_BYTE(m_inputRegister[BSS_MODBUS_CAB_POGO_TEMP_12_REG_INDEX]);

    _info.m_pogoTemps[2] = MODBUS_RTU_REG_GET_LOW_BYTE(m_inputRegister[BSS_MODBUS_CAB_POGO_TEMP_34_REG_INDEX]);
    _info.m_pogoTemps[3] = MODBUS_RTU_REG_GET_HIGH_BYTE(m_inputRegister[BSS_MODBUS_CAB_POGO_TEMP_34_REG_INDEX]);

    uint8_t bpConnectionState = MODBUS_RTU_REG_GET_LOW_BYTE(m_inputRegister[BSS_MODBUS_BP_CONNECTED_STATE_REG_INDEX]);

    std::lock_guard<std::mutex> lock{m_bpLockQueue};
    std::vector<uint16_t> bpData(m_inputRegister,
                                 m_inputRegister + BSS_MODBUS_SYNC_REG_NUMBER);

    auto modbusData = std::make_shared<ModbusBpData>(_id, bpData);
    if(bpConnectionState){
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        uint16_t bpSnArray[DEVICE_SN_LENGTH / 2] = {0,};
        ret = m_modbusMaster->readInputRegisters(_id + m_idOffset,
                                                 MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_BP_SN),
                                                 DEVICE_SN_LENGTH / 2,
                                                 bpSnArray);
        if(ret != MODBUS_ERROR_NONE){
            LOG_ERROR(TAG,"Could NOT read BP serial at cabinet: %d", _id);
            return -1;
        }else{
            modbusData->m_sn.insert(modbusData->m_sn.begin(), bpSnArray, &bpSnArray[DEVICE_SN_LENGTH/2]);
        }
    }
    m_bpsUpdatedData.push(modbusData);

    m_trigger.notify_all();

    return 0;
}


//******************************* BP Communication ***********************//
void BssModbusCom::addManager(std::shared_ptr<BpManagerInterface> _manager) {
    m_bpManager = _manager;
}

bool BssModbusCom::sync(bool _enable) {
    return false;
}

int32_t BssModbusCom::assign(const std::shared_ptr<Bp>& _bp) {
    std::lock_guard<std::mutex> lockGuard(m_lock);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    int32_t ret = m_modbusMaster->writeSingleRegister(_bp->m_id + m_idOffset,
                                                      MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_BP_ASSIGNING),
                                                      BSS_MODBUS_TURN_ON);
    if(ret != MODBUS_ERROR_NONE){
        LOG_ERROR(TAG, "Command Assign BP %d FAILURE", _bp->m_id);
        return -1;
    }
    return 0;
}

int32_t BssModbusCom::setCmd(const std::shared_ptr<Bp::BpCmd>& _cmd) {
    std::lock_guard<std::mutex> lockGuard(m_lock);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    int32_t ret = -1;
    m_modbusMaster->setResponseTimeout(1000);
    switch (_cmd->m_cmd) {
        case BP_COMMAND::BP_CMD_REBOOT:
            ret = m_modbusMaster->writeSingleRegister(_cmd->m_bp->m_id + m_idOffset,
                                                      MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_BP_REBOOT),
                                                      BSS_MODBUS_TURN_ON);
            break;
        case BP_COMMAND::BP_CMD_CHARGED:
            ret = m_modbusMaster->writeSingleRegister(_cmd->m_bp->m_id + m_idOffset,
                                                      MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_BP_CHARGER_SWITCH),
                                                      BSS_MODBUS_TURN_ON);
            break;
        case BP_COMMAND::BP_CMD_DISCHARGED:
            ret = m_modbusMaster->writeSingleRegister(_cmd->m_bp->m_id + m_idOffset,
                                                      MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_BP_CHARGER_SWITCH),
                                                      BSS_MODBUS_TURN_OFF);
            break;
        case BP_COMMAND::BP_CMD_WRITE_ASSIGNED_DEVICE:
            ret = m_modbusMaster->writeMultipleRegisters(_cmd->m_bp->m_id + m_idOffset,
                                                      MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_BP_ASSIGNED_DEVICE),
                                                      16,
                                                      (uint16_t*)_cmd->m_data);
            break;
        case BP_COMMAND::BP_CMD_READ_SN:
            ret = m_modbusMaster->readInputRegisters(_cmd->m_bp->m_id + m_idOffset,
                                                         MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_BP_SN),
                                                         16,
                                                         (uint16_t*)_cmd->m_data);
            break;
        case BP_COMMAND::BP_CMD_READ_VERSION:
            ret = m_modbusMaster->readInputRegisters(_cmd->m_bp->m_id + m_idOffset,
                                                         MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_BP_VERSION),
                                                         16,
                                                         (uint16_t*)_cmd->m_data);
            break;
        case BP_COMMAND::BP_CMD_READ_ASSIGNED_DEVICE:
            ret = m_modbusMaster->readHoldingRegisters(_cmd->m_bp->m_id + m_idOffset,
                                                         MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_BP_ASSIGNED_DEVICE),
                                                         16,
                                                         (uint16_t*)_cmd->m_data);
            break;
        default:
            LOG_ERROR(TAG, "BP Command is NOT support");
            return -1;
    }


    if(ret != MODBUS_ERROR_NONE){
        LOG_ERROR(TAG, "Set command %s on BP %d FAILURE", convertBpCmdToString(_cmd->m_cmd).c_str(), _cmd->m_bp->m_id);
        if(_cmd->m_callback){
            _cmd->m_callback(_cmd->m_cmd,
                             BP_CMD_FAILURE,
                             _cmd->m_bp->m_id,
                             _cmd->m_data,
                             _cmd->m_arg);
        }
        return -1;
    }

    if(_cmd->m_callback){
        _cmd->m_callback(_cmd->m_cmd,
                         BP_CMD_SUCCESS,
                         _cmd->m_bp->m_id,
                         _cmd->m_data,
                         _cmd->m_arg);
    }

    m_bpManager->resetCurrentCmd();

    return 0;
}

int32_t BssModbusCom::resetCmd() {
    return 0;
}

bool BssModbusCom::isBusy() {
    return false;
}

int32_t BssModbusCom::process() {
    std::unique_lock<std::mutex> lock(m_bpLockQueue);
    m_trigger.wait(lock, [&]() { return !m_bpsUpdatedData.empty(); });

    auto bpData = m_bpsUpdatedData.front();
    m_bpsUpdatedData.pop();

    auto bp = m_bpManager->getBp(bpData->m_id);
    uint8_t bpConnectionState = MODBUS_RTU_REG_GET_LOW_BYTE(bpData->m_data[BSS_MODBUS_BP_CONNECTED_STATE_REG_INDEX]);

    if(bpConnectionState){
        if(bpData->m_sn.empty()){
            return -1;
        }

        std::string sn = decodeBpStringValue(bpData->m_sn);
        std::reverse(sn.begin(), sn.end());

        decodeBpValue(bp, bpData->m_data);

        if (bp->m_info.m_sn.empty() && bp->m_assignStatus != BP_ASSIGN_STATE::BP_ASSIGNED) {
            bp->m_info.m_sn = sn;
            bp->m_assignStatus = BP_ASSIGN_STATE::BP_ASSIGNED;

            LOG_INFO(TAG, "New BP %s is assigned at cabinet: %d, Notify to observer ", bp->m_info.m_sn.c_str(), bp->m_id);

            if(m_bpManager) {
                m_bpManager->notifyAssigningState(bp->m_id,
                                                  BP_ASSIGN_STATE::BP_ASSIGNED,
                                                  bp->m_info.m_sn);
            }
            return 0;
        }

        if(sn != bp->m_info.m_sn){
            LOG_WARN(TAG, "Old BP %s is disconnected at cabinet: %d ", bp->m_info.m_sn.c_str(), bp->m_id);
            if(m_bpManager) {
                m_bpManager->notifyAssigningState(bp->m_id,
                                                  BP_ASSIGN_STATE::BP_ASSIGN_FAILURE,
                                                  bp->m_info.m_sn,
                                                  bp->m_info.m_version);
            }
            m_bpManager->resetBp(bp->m_id);
        }
    }else{
        if (bp->m_assignStatus == BP_ASSIGN_STATE::BP_ASSIGNED) {
            if (m_bpManager) {
                m_bpManager->notifyAssigningState(bpData->m_id,
                                                  BP_ASSIGN_STATE::BP_ASSIGN_FAILURE,
                                                  bp->m_info.m_sn,
                                                  bp->m_info.m_version);
            }
            LOG_WARN(TAG, "Reset BP %d, Reason BP %s is disconnected", bp->m_id, bp->m_info.m_sn.c_str());
            m_bpManager->resetBp(bp->m_id);
        }
    }

    return 0;
}

std::string BssModbusCom::decodeBpStringValue(const std::vector<uint16_t> &_regValue) {
    char arrayValue[DEVICE_SN_LENGTH];
    uint8_t index = 0;
    for(uint16_t value : _regValue){
        arrayValue[index++] = MODBUS_RTU_REG_GET_LOW_BYTE(value);
        arrayValue[index++] = MODBUS_RTU_REG_GET_HIGH_BYTE(value);
    }
    arrayValue[index] = '\0';

    return std::string{arrayValue};
}

void BssModbusCom::decodeBpValue(const std::shared_ptr<Bp>& _bp,  const std::vector<uint16_t>&  _regValue){
    const std::shared_ptr<Bp>& bp = _bp;

    bp->m_info.m_status = _regValue[BSS_MODBUS_BP_STATUS_REG_INDEX];
    bp->m_info.m_state = _regValue[BSS_MODBUS_BP_STATE_REG_INDEX ];
    bp->m_info.m_cycle = _regValue[BSS_MODBUS_BP_CYCLE_REG_INDEX ];

    bp->m_info.m_vol = _regValue[BSS_MODBUS_BP_VOL_REG_INDEX];
    bp->m_info.m_cur = _regValue[BSS_MODBUS_BP_CUR_REG_INDEX];

    bp->m_info.m_soc = MODBUS_RTU_REG_GET_HIGH_BYTE(_regValue[BSS_MODBUS_BP_SOC_SOH_REG_INDEX]);
    bp->m_info.m_soh = MODBUS_RTU_REG_GET_LOW_BYTE(_regValue[BSS_MODBUS_BP_CYCLE_REG_INDEX]);

    bp->m_info.m_cellVols[0] = _regValue[BSS_MODBUS_BP_CELL_0_REG_INDEX];
    bp->m_info.m_cellVols[1] = _regValue[BSS_MODBUS_BP_CELL_1_REG_INDEX];
    bp->m_info.m_cellVols[2] = _regValue[BSS_MODBUS_BP_CELL_2_REG_INDEX];
    bp->m_info.m_cellVols[3] = _regValue[BSS_MODBUS_BP_CELL_3_REG_INDEX];
    bp->m_info.m_cellVols[4] = _regValue[BSS_MODBUS_BP_CELL_4_REG_INDEX];
    bp->m_info.m_cellVols[5] = _regValue[BSS_MODBUS_BP_CELL_5_REG_INDEX];
    bp->m_info.m_cellVols[6] = _regValue[BSS_MODBUS_BP_CELL_6_REG_INDEX];
    bp->m_info.m_cellVols[7] = _regValue[BSS_MODBUS_BP_CELL_7_REG_INDEX];
    bp->m_info.m_cellVols[8] = _regValue[BSS_MODBUS_BP_CELL_8_REG_INDEX];
    bp->m_info.m_cellVols[9] = _regValue[BSS_MODBUS_BP_CELL_9_REG_INDEX];
    bp->m_info.m_cellVols[10] = _regValue[BSS_MODBUS_BP_CELL_10_REG_INDEX];
    bp->m_info.m_cellVols[11] = _regValue[BSS_MODBUS_BP_CELL_11_REG_INDEX];
    bp->m_info.m_cellVols[12] = _regValue[BSS_MODBUS_BP_CELL_12_REG_INDEX];
    bp->m_info.m_cellVols[13] = _regValue[BSS_MODBUS_BP_CELL_13_REG_INDEX];
    bp->m_info.m_cellVols[14] = _regValue[BSS_MODBUS_BP_CELL_14_REG_INDEX];
    bp->m_info.m_cellVols[15] = _regValue[BSS_MODBUS_BP_CELL_15_REG_INDEX];

    bp->m_info.m_temps[0] = MODBUS_RTU_REG_GET_LOW_BYTE(_regValue[BSS_MODBUS_BP_ZONE_TEMP_01_REG_INDEX]);
    bp->m_info.m_temps[1] = MODBUS_RTU_REG_GET_HIGH_BYTE(_regValue[BSS_MODBUS_BP_ZONE_TEMP_01_REG_INDEX]);
    bp->m_info.m_temps[2] = MODBUS_RTU_REG_GET_LOW_BYTE(_regValue[BSS_MODBUS_BP_ZONE_TEMP_23_REG_INDEX]);
    bp->m_info.m_temps[3] = MODBUS_RTU_REG_GET_HIGH_BYTE(_regValue[BSS_MODBUS_BP_ZONE_TEMP_01_REG_INDEX]);
    bp->m_info.m_temps[4] = MODBUS_RTU_REG_GET_LOW_BYTE(_regValue[BSS_MODBUS_BP_ZONE_TEMP_45_REG_INDEX]);
    bp->m_info.m_temps[5] = MODBUS_RTU_REG_GET_HIGH_BYTE(_regValue[BSS_MODBUS_BP_ZONE_TEMP_01_REG_INDEX]);
    bp->m_info.m_temps[6] = MODBUS_RTU_REG_GET_LOW_BYTE(_regValue[BSS_MODBUS_BP_ZONE_TEMP_6_REG_INDEX]);

    bp->m_timeout.reset();

//    if(bp->m_id == 4){
        LOG_DEBUG(TAG, "BP %d current = %d",bp->m_id,  _regValue[BSS_MODBUS_BP_CUR_REG_INDEX]);
//    }
}