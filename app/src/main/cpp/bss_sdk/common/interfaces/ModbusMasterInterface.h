//
// Created by vnbk on 05/09/2023.
//

#ifndef BSS_SDK_MODBUSINTERFACE_H
#define BSS_SDK_MODBUSINTERFACE_H

#include <memory>
#include <utility>

#include "ModbusDefine.h"
#include "ModbusHandleInterface.h"
#include "ModbusComInterface.h"

class ModbusMasterInterface{
public:
    ModbusMasterInterface(std::shared_ptr<ModbusComInterface>& _comIf,
                          std::shared_ptr<ModbusHandleInterface> _handle) : m_comIf(_comIf),
                          m_handle(std::move(_handle)){

    }

    virtual ~ModbusMasterInterface() = default;

    int32_t addHandle(std::shared_ptr<ModbusHandleInterface>& _handle){
        m_handle = _handle;
        return 0;
    }

    virtual int32_t open(int32_t _baudRate) = 0;

    virtual int32_t close() = 0;

    virtual int32_t setBaudRateRtu(int32_t _baudRate) = 0;

    virtual int32_t getBaudRateRtu() = 0;

    virtual int32_t getResponseTimeout() = 0;

    virtual void setResponseTimeout(int32_t _timeout) = 0;

    virtual int32_t readCoils(const uint16_t& _slaveAddr, uint16_t _startAddr, uint16_t _quantity, uint8_t* _coilsOut) = 0;

    virtual int32_t readDiscreteInputs(const uint16_t& _slaveAddr, uint16_t _startAddr, uint16_t _quantity, uint8_t* _inputsOut) = 0;

    virtual int32_t readHoldingRegisters(const uint16_t& _slaveAddr, uint16_t _startAddr, uint16_t _quantity, uint16_t* _registersOut) = 0;

    virtual int32_t readInputRegisters(const uint16_t& _slaveAddr, uint16_t _startAddr, uint16_t _quantity, uint16_t* _registersOut) = 0;

    virtual int32_t writeSingleCoil(const uint16_t& _slaveAddr, uint16_t _startAddr, bool value) = 0;

    virtual int32_t writeSingleRegister(const uint16_t& _slaveAddr, uint16_t _startAddr, uint16_t _value) = 0;

    virtual int32_t writeMultipleCoils(const uint16_t& _slaveAddr, uint16_t _startAddr, uint16_t _quantity, const uint8_t* _coils) = 0;

    virtual int32_t writeMultipleRegisters(const uint16_t& _slaveAddr, uint16_t _startAddr, uint16_t _quantity, const uint16_t* _registers) = 0;

    virtual int32_t readFileRecord(const uint16_t& _slaveAddr, uint16_t _fileNumber, uint16_t _recordNumber, uint16_t* _registers, uint16_t _count) = 0;

    virtual int32_t writeFileRecord(const uint16_t& _slaveAddr, uint16_t _fileNumber, uint16_t _recordNumber, const uint16_t* _registers, uint16_t _count) = 0;

    virtual void process() = 0;

protected:
    std::shared_ptr<ModbusHandleInterface> m_handle;
    std::shared_ptr<ModbusComInterface> m_comIf;
};

#endif //BSS_SDK_MODBUSINTERFACE_H
