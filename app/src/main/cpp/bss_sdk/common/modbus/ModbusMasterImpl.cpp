//
// Created by vnbk on 18/09/2023.
//

#include "ModbusMasterImpl.h"

#include <utility>
#include "Logger.h"

#define TAG "ModbusMasterImpl"

static int32_t nmbs_platform_read(uint8_t* buf,
                                  uint16_t count,
                                  int32_t byte_timeout_ms,
                                  void* arg){
    auto modbusCom = (ModbusComInterface*)arg;
    if(!modbusCom){
        return -1;
    }
    return modbusCom->recv(buf, count, byte_timeout_ms);
}

static int32_t nmbs_platform_write(const uint8_t* buf,
                                   uint16_t count,
                                   int32_t byte_timeout_ms,
                                   void* arg){
    auto modbusCom = (ModbusComInterface*)arg;
    if(!modbusCom){
        return -1;
    }
    return modbusCom->send(buf, count, byte_timeout_ms);
}

std::shared_ptr<ModbusMasterImpl> ModbusMasterImpl::create(std::shared_ptr<ModbusComInterface> &_comIf,
                                                           std::shared_ptr<ModbusHandleInterface> _handle) {
    auto modbusMaster = std::shared_ptr<ModbusMasterImpl>(new ModbusMasterImpl(_comIf, std::move(_handle)));
    if(modbusMaster->initialized()){
        LOG_INFO(TAG, "Created Modbus Master SUCCESS");
        return modbusMaster;
    }

    return nullptr;
}

ModbusMasterImpl::ModbusMasterImpl(std::shared_ptr<ModbusComInterface>& _comIf,
                                   std::shared_ptr<ModbusHandleInterface> _handle) : ModbusMasterInterface(_comIf, std::move(_handle)) {
    m_nanoModbusPlatform.transport = NMBS_TRANSPORT_RTU;
    m_nanoModbusPlatform.write = nmbs_platform_write;
    m_nanoModbusPlatform.read = nmbs_platform_read;
    m_nanoModbusPlatform.arg = m_comIf.get();
}

ModbusMasterImpl::~ModbusMasterImpl() noexcept {
}

bool ModbusMasterImpl::initialized() {
    if(!m_comIf){
        LOG_ERROR(TAG, "Missing PARAMETERS, please check again");
        return false;
    }

    nmbs_error err = nmbs_client_create(&m_nanoModbus, &m_nanoModbusPlatform);

    nmbs_set_byte_timeout(&m_nanoModbus, MODBUS_RESPONSE_TIMEOUT_DEFAULT);
    nmbs_set_read_timeout(&m_nanoModbus, MODBUS_RESPONSE_TIMEOUT_DEFAULT);

    if (err != NMBS_ERROR_NONE) {
        LOG_ERROR(TAG, "Error creating modbus server\n");
        return false;
    }

    return true;
}

int32_t ModbusMasterImpl::open(int32_t _baudRate) {
    if(m_comIf){
        int ret = m_comIf->open(_baudRate);
        if(ret < 0){
            LOG_ERROR(TAG, "Modbus Master open interface FAILURE");
            return -1;
        }
        return ret;
    }
    return -1;
}

int32_t ModbusMasterImpl::close() {
    if(m_comIf){
        return m_comIf->close();
    }
    return -1;
}

int32_t ModbusMasterImpl::setBaudRateRtu(int32_t _baudRate) {
    if(m_comIf->getBaudRate() != _baudRate){
        return m_comIf->config(_baudRate);
    }
    return 0;
}

int32_t ModbusMasterImpl::getBaudRateRtu() {
    return m_comIf->getBaudRate();
}

int32_t ModbusMasterImpl::getResponseTimeout(){
    return m_nanoModbus.read_timeout_ms;
}

void ModbusMasterImpl::setResponseTimeout(int32_t _timeout){
    nmbs_set_byte_timeout(&m_nanoModbus, _timeout);
    nmbs_set_read_timeout(&m_nanoModbus, _timeout);
}

int32_t ModbusMasterImpl::readCoils(const uint16_t &_slaveAddr,
                                    uint16_t _startAddr,
                                    uint16_t _quantity,
                                    uint8_t *_coilsOut) {
    nmbs_set_destination_rtu_address(&m_nanoModbus, _slaveAddr);
    uint8_t coils[MODBUS_MAX_COILS_SUPPORT];
    nmbs_error err = nmbs_read_coils(&m_nanoModbus, _startAddr, _quantity, coils);
    if(err == NMBS_ERROR_NONE){
        memcpy(_coilsOut, coils, MODBUS_MAX_COILS_SUPPORT);
    } else{
        LOG_ERROR(TAG, "Read coils ERROR: %d", err);
    }

    return err;
}

int32_t ModbusMasterImpl::readDiscreteInputs(const uint16_t &_slaveAddr,
                                             uint16_t _startAddr,
                                             uint16_t _quantity,
                                             uint8_t *_inputsOut) {
    nmbs_set_destination_rtu_address(&m_nanoModbus, _slaveAddr);
    uint8_t inputs[MODBUS_MAX_COILS_SUPPORT];
    nmbs_error err = nmbs_read_discrete_inputs(&m_nanoModbus, _startAddr, _quantity, inputs);
    if(err == NMBS_ERROR_NONE){
        memcpy(_inputsOut, inputs, MODBUS_MAX_COILS_SUPPORT);
    }else{
        LOG_ERROR(TAG, "Read Discrete Inputs ERROR: %d", err);
    }

    return err;
}

int32_t ModbusMasterImpl::readHoldingRegisters(const uint16_t &_slaveAddr,
                                               uint16_t _startAddr,
                                               uint16_t _quantity,
                                               uint16_t *_registersOut) {
    nmbs_set_destination_rtu_address(&m_nanoModbus, _slaveAddr);
    nmbs_error err = nmbs_read_holding_registers(&m_nanoModbus, _startAddr, _quantity, _registersOut);

    if(err != NMBS_ERROR_NONE){
        LOG_ERROR(TAG, "Read holding registers ERROR: %d", err);
    }

    return err;
}

int32_t ModbusMasterImpl::readInputRegisters(const uint16_t &_slaveAddr,
                                             uint16_t _startAddr,
                                             uint16_t _quantity,
                                             uint16_t *_registersOut) {
    nmbs_set_destination_rtu_address(&m_nanoModbus, _slaveAddr);
    nmbs_error err = nmbs_read_input_registers(&m_nanoModbus, _startAddr, _quantity, _registersOut);

    if(err != NMBS_ERROR_NONE){
        LOG_ERROR(TAG, "Read Input registers ERROR: %d", err);
    }

    return err;
}

int32_t ModbusMasterImpl::writeSingleCoil(const uint16_t &_slaveAddr,
                                          uint16_t _startAddr,
                                          bool _value) {
    nmbs_set_destination_rtu_address(&m_nanoModbus, _slaveAddr);
    nmbs_error err = nmbs_write_single_coil(&m_nanoModbus, _startAddr, _value);

    if(err != NMBS_ERROR_NONE){
        LOG_ERROR(TAG, "Write single coil ERROR: %d", err);
    }

    return err;
}

int32_t ModbusMasterImpl::writeSingleRegister(const uint16_t &_slaveAddr,
                                              uint16_t _startAddr,
                                              uint16_t _value) {
    nmbs_set_destination_rtu_address(&m_nanoModbus, _slaveAddr);
    nmbs_error err = nmbs_write_single_register(&m_nanoModbus, _startAddr, _value);

    if(err != NMBS_ERROR_NONE){
        LOG_ERROR(TAG, "Write single register ERROR: %d", err);
    }

    return err;
}

int32_t ModbusMasterImpl::writeMultipleCoils(const uint16_t &_slaveAddr,
                                             uint16_t _startAddr,
                                             uint16_t _quantity,
                                             const uint8_t *_coils) {
    nmbs_set_destination_rtu_address(&m_nanoModbus, _slaveAddr);
    nmbs_error err = nmbs_write_multiple_coils(&m_nanoModbus, _startAddr, _quantity, _coils);

    if(err != NMBS_ERROR_NONE){
        LOG_ERROR(TAG, "Write write multiple coils ERROR: %d", err);
    }

    return err;
}

int32_t ModbusMasterImpl::writeMultipleRegisters(const uint16_t &_slaveAddr,
                                                 uint16_t _startAddr,
                                                 uint16_t _quantity,
                                                 const uint16_t *_registers) {
    nmbs_set_destination_rtu_address(&m_nanoModbus, _slaveAddr);
    nmbs_error err = nmbs_write_multiple_registers(&m_nanoModbus, _startAddr, _quantity, _registers);

    if(err != NMBS_ERROR_NONE){
        LOG_ERROR(TAG, "Write write multiple registers ERROR: %d", err);
    }

    return err;
}

int32_t ModbusMasterImpl::readFileRecord(const uint16_t &_slaveAddr,
                                         uint16_t _fileNumber,
                                         uint16_t _recordNumber,
                                         uint16_t *_registers,
                                         uint16_t _count) {
    nmbs_set_destination_rtu_address(&m_nanoModbus, _slaveAddr);
    nmbs_error err = nmbs_read_file_record(&m_nanoModbus, _fileNumber, _recordNumber, _registers, _count);

    if(err != NMBS_ERROR_NONE){
        LOG_ERROR(TAG, "Read file record ERROR: %d", err);
    }

    return err;
}

int32_t ModbusMasterImpl::writeFileRecord(const uint16_t &_slaveAddr,
                                          uint16_t _fileNumber,
                                          uint16_t _recordNumber,
                                          const uint16_t *_registers,
                                          uint16_t _count) {
    nmbs_set_destination_rtu_address(&m_nanoModbus, _slaveAddr);
    nmbs_error err = nmbs_write_file_record(&m_nanoModbus, _fileNumber, _recordNumber, _registers, _count);

    if(err != NMBS_ERROR_NONE){
        LOG_ERROR(TAG, "Write file record ERROR: %d", err);
    }

    return err;
}

void ModbusMasterImpl::process() {
//    m_comIf->process();
}