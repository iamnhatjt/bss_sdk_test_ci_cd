//
// Created by vnbk on 24/07/2023.
//

#include "ModbusEMeterManager.h"

#include <utility>
#include "Logger.h"

#define TAG "ModbusEMeterManager"

std::shared_ptr<ModbusEMeterManager> ModbusEMeterManager::create(std::shared_ptr<ModbusMasterInterface>& _modbusMaster,
                                                                 int32_t _periodUpdate) {
    auto eMeter = std::shared_ptr<ModbusEMeterManager>(new ModbusEMeterManager(_modbusMaster, _periodUpdate));
    if(eMeter->initialized()){
        LOG_INFO(TAG, "Created ModbusEMeterManager SUCCESS");
        return eMeter;
    }
    LOG_ERROR(TAG, "Created ModbusEMeterManager FAILURE");
    return nullptr;
}

ModbusEMeterManager::~ModbusEMeterManager() noexcept {
    m_modbusRtuEMeterSupport.clear();
}

ModbusEMeterManager::ModbusEMeterManager(std::shared_ptr<ModbusMasterInterface>& _modbusMaster,
                                         int32_t _periodUpdate) : m_modbusMaster(std::move(_modbusMaster)),
                                            m_periodUpdate(_periodUpdate),
                                            m_periodUpdateTimeout(m_periodUpdate),
                                            m_retry(0){
    m_info.m_vol = 0;
    m_info.m_cur = 0;
    m_info.m_cos = 0;
    m_info.m_freq = 0;
    m_info.m_kwh = 0;
}

bool ModbusEMeterManager::initialized() {
    if(!m_modbusMaster){
        LOG_ERROR(TAG, "Modbus Master is NULL, Please check again");
        return false;
    }
    m_periodUpdateTimeout.reset();

    return true;
}

const std::string ModbusEMeterManager::getManufactureActive() {
    if(m_activeEMeter){
        return m_activeEMeter->getManufacture();
    }
    return "";
}

void ModbusEMeterManager::addModbusRtuEMeterSupport(std::shared_ptr<ModbusRtuEMeterInterface> &_modbusEMeter) {
    m_modbusRtuEMeterSupport.push_back(_modbusEMeter);
}

void ModbusEMeterManager::setReadingPeriod(int32_t _period) {
    m_periodUpdate = _period;
    m_periodUpdateTimeout.reset(m_periodUpdate);
}

EMeterManagerInterface::EMeterInfo& ModbusEMeterManager::getAll(){
    return m_info;
}

float ModbusEMeterManager::getVol(){
    return m_info.m_vol;
}

float ModbusEMeterManager::getCur(){
    return m_info.m_cur;
}

float ModbusEMeterManager::getCos(){
    return m_info.m_cos;
}

float ModbusEMeterManager::getFreq(){
    return m_info.m_freq;
}

float ModbusEMeterManager::getKwh(){
    return m_info.m_kwh;
}

int32_t ModbusEMeterManager::updateValue(const uint16_t &_startAddr, const uint16_t &_quantity, const uint16_t *_reg) {
    int emeterIndex = -1;
    for(int index = 0; index <= _quantity; index+=2){
        emeterIndex = m_activeEMeter->getIndexByAddress(_startAddr + index);
        if(emeterIndex < 0){
            continue;
        }

        if(updateValue(emeterIndex, _reg + index)){
            return -1;
        }
    }
    return 0;
}

int32_t ModbusEMeterManager::updateValue(uint8_t _paramIndex, const uint16_t*  _regValue) {
    float newValue;

    if(m_activeEMeter->decodeValue(_paramIndex, _regValue, newValue) < 0){
        LOG_ERROR(TAG, "Value %s EMeter is WRONG", emeterParamToString(_paramIndex).c_str());
        m_activeEMeter.reset();
        return -1;
    }

    switch (_paramIndex) {
        case ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_VOLTAGE:
            m_info.m_vol = newValue;
            break;
        case ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_CURRENT:
            m_info.m_cur = newValue;
            break ;
        case ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_COS:
            m_info.m_cos = newValue;
            break ;
        case ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_FREQ:
            m_info.m_freq = newValue;
            break ;
        case ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_ENERGY:
            m_info.m_kwh = newValue;
            break ;
        default:
            LOG_WARN(TAG, "EMeter params INVALID");
            return -1;
    }
    LOG_DEBUG(TAG, "Emeter %s: %f", emeterParamToString(_paramIndex).c_str(), newValue);
    return 0;
}

void ModbusEMeterManager::reset() {
    m_info.m_vol = 0;
    m_info.m_cur = 0;
    m_info.m_cos = 0;
    m_info.m_freq = 0;
    m_info.m_kwh = 0;
    m_activeEMeter.reset();
}

int32_t ModbusEMeterManager::readEMeterParam(const std::shared_ptr<ModbusRtuEMeterInterface> &_emeter) {
    MODBUS_FUNCTION_CODE code;
    uint16_t reg = 0;
    uint16_t regValue[20] = {};
    uint16_t quantity = 0;
    int32_t ret = -1;

    int32_t baudRate = _emeter->getBaudRate();
    int32_t currentBaud = m_modbusMaster->getBaudRateRtu();
    if(baudRate != currentBaud){
        LOG_DEBUG(TAG, "Changing baud rate to read EMeter from: %d to %d", currentBaud, baudRate);
        if(m_modbusMaster->setBaudRateRtu(baudRate) < 0){
            LOG_ERROR(TAG, "Could NOT change baud rate for Modbus RTU EMeter");
            return -1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); /// For COM is ready
    }

    int32_t emeterResponseTime = _emeter->getResponseTimeout();
    int32_t currentResponseTime = m_modbusMaster->getResponseTimeout();
    if(emeterResponseTime != currentResponseTime){
        LOG_DEBUG(TAG, "Change response time to read EMeter from: %d to %d", currentResponseTime, emeterResponseTime);
        m_modbusMaster->setResponseTimeout(emeterResponseTime);
    }

    LOG_DEBUG(TAG, "Read EMeter info from: %s", _emeter->getManufacture().c_str());

    if(_emeter->readContinuous(code, reg, quantity)){
        switch (code) {
            case MODBUS_FUNCTION_CODE::READ_HOLDING_REGISTERS:
                if(!m_modbusMaster->readHoldingRegisters(_emeter->getAddress(), reg, quantity, (uint16_t*)&regValue)){
                    if(!m_activeEMeter){
                        m_activeEMeter = _emeter;
                        LOG_INFO(TAG, "Modbus RTU EMeter %s is active", _emeter->getManufacture().c_str());
                    }
                    if(updateValue(reg, quantity, regValue)){
                        break;
                    }
                    ret = 0;
                }
                break;
            case MODBUS_FUNCTION_CODE::READ_INPUT_REGISTERS:
                break;
            default:
                LOG_WARN(TAG, "EMeter %s NOT Support", _emeter->getManufacture().c_str());
                break;
        }
        goto exit;
    }

    for(int index = 0; index < ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_NUMBER_INDEX; index++){
        uint32_t value = _emeter->getProfile(index);
        if(!value){
            continue;
        }
        code = (MODBUS_FUNCTION_CODE)MODBUS_RTU_DECODE_FUNCTION_CODE(value);
        reg = MODBUS_RTU_DECODE_REGISTER(value);

        switch (code) {
            case MODBUS_FUNCTION_CODE::READ_HOLDING_REGISTERS:
                if(!m_modbusMaster->readHoldingRegisters(_emeter->getAddress(), reg, 2, (uint16_t*)&regValue)){
                    if(!m_activeEMeter){
                        m_activeEMeter = _emeter;
                        LOG_INFO(TAG, "Modbus RTU EMeter %s is active", _emeter->getManufacture().c_str());
                    }
                    if(updateValue(index, regValue) < 0){
                        break;
                    }
                    ret = 0;
                }
                break;
            case MODBUS_FUNCTION_CODE::READ_INPUT_REGISTERS:
                if(!m_modbusMaster->readInputRegisters(_emeter->getAddress(), reg, 2, (uint16_t*)&regValue)){
                    if(!m_activeEMeter){
                        m_activeEMeter = _emeter;
                        LOG_INFO(TAG, "Modbus RTU EMeter %s is active", _emeter->getManufacture().c_str());
                    }

                    if(updateValue(index, regValue) < 0){
                        break;
                    }
                    ret = 0;
                }
                break;
            default:
                LOG_WARN(TAG, "Not support function CODE: 0x%2X", code);
        }
    }

    exit:
    if (baudRate != currentBaud) {
        LOG_WARN(TAG, "Update current Baud Rate from %d to %d", baudRate, currentBaud);
        if (m_modbusMaster->setBaudRateRtu(currentBaud) < 0) {
            LOG_ERROR(TAG, "Could NOT change baud rate for other interface");
            return -1;
        }
        if (emeterResponseTime != currentResponseTime) {
            LOG_DEBUG(TAG, "Change response time to read EMeter from: %d to %d", emeterResponseTime, currentResponseTime);
            m_modbusMaster->setResponseTimeout(currentResponseTime);
        }
    }
    return ret;
}

void ModbusEMeterManager::process() {
    if(!m_periodUpdateTimeout.getRemainTime()){
        if (m_activeEMeter) {
            if(!readEMeterParam(m_activeEMeter)){
                m_retry = 0;
            }else{
                m_retry++;
            }
            if(m_retry >= CHECK_EMETER_ACTIVE_TIME_RETRIED){
                LOG_WARN(TAG, "Active EMeter %s is offline, Please check again", m_activeEMeter->getManufacture().c_str());
                reset();
            }

        } else {
            for (auto &item: m_modbusRtuEMeterSupport) {
                if(!readEMeterParam(item))
                    break;
            }
        }
        m_periodUpdateTimeout.reset();
    }
}