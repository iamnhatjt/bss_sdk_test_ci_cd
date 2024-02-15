//
// Created by vnbk on 18/09/2023.
//
#include "ModbusComImpl.h"
#include "Logger.h"
#include "Timer.h"

#define TAG "ModbusComImpl"

std::shared_ptr<ModbusComImpl> ModbusComImpl::create(std::shared_ptr<SerialPort>& _serialPort) {
    auto modbusCom = std::shared_ptr<ModbusComImpl>(new ModbusComImpl(_serialPort));
    if(modbusCom->initialized()){
        LOG_INFO(TAG, "Created Modbus Com SUCCESS");
        return modbusCom;
    }
    return nullptr;
}

ModbusComImpl::ModbusComImpl(std::shared_ptr<SerialPort>& _serialPort) : m_serialPort(_serialPort){

}

bool ModbusComImpl::initialized() {
    return true;
}

ModbusComImpl::~ModbusComImpl() noexcept {
    if(m_serialPort->isOpen()){
        m_serialPort->close();
    }
}

int32_t ModbusComImpl::open(int32_t _baudRate) {
    return m_serialPort->open(_baudRate);
}

int32_t ModbusComImpl::close() {
    return m_serialPort->close();
}

int32_t ModbusComImpl::config(int32_t _baudRate) {
    return m_serialPort->config(_baudRate);
}

int32_t ModbusComImpl::getBaudRate() {
    return m_serialPort->getBaudRate();
}

int32_t ModbusComImpl::send(const void *_data, int32_t _len, int32_t _timeout) {
    m_serialPort->flush();
    return m_serialPort->send((const uint8_t*)_data, _len, _timeout);
}

int32_t ModbusComImpl::recv(void *_data, int32_t _maxLen, int32_t _timeout) {
    WaitTimer timeout(_timeout);

    while (timeout.getRemainTime() && m_serialPort->dataIsAvailable() < _maxLen);

    int32_t len = (_maxLen >= m_serialPort->dataIsAvailable()) ? m_serialPort->dataIsAvailable() : _maxLen;

    return m_serialPort->receive((uint8_t*)_data, len);
}