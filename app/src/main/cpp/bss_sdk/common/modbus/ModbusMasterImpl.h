//
// Created by vnbk on 18/09/2023.
//

#ifndef BSS_SDK_MODBUSMASTERIMPL_H
#define BSS_SDK_MODBUSMASTERIMPL_H

#include "ModbusMasterInterface.h"
#include "nanomodbus.h"

class ModbusMasterImpl : public ModbusMasterInterface{
public:
    static std::shared_ptr<ModbusMasterImpl> create(std::shared_ptr<ModbusComInterface>& _comIf,
                                                    std::shared_ptr<ModbusHandleInterface> _handle = nullptr);

    ~ModbusMasterImpl() override;

    int32_t open(int32_t _baudRate) override;

    int32_t close() override;

    int32_t setBaudRateRtu(int32_t _baudRate) override;

    int32_t getBaudRateRtu() override;

    int32_t getResponseTimeout() override;

    void setResponseTimeout(int32_t _timeout) override;

    int32_t readCoils(const uint16_t& _slaveAddr, uint16_t _startAddr, uint16_t _quantity, uint8_t* _coilsOut) override;

    int32_t readDiscreteInputs(const uint16_t& _slaveAddr, uint16_t _startAddr, uint16_t _quantity, uint8_t* _inputsOut) override;

    int32_t readHoldingRegisters(const uint16_t& _slaveAddr, uint16_t _startAddr, uint16_t _quantity, uint16_t* _registersOut) override;

    int32_t readInputRegisters(const uint16_t& _slaveAddr, uint16_t _startAddr, uint16_t _quantity, uint16_t* _registersOut) override;

    int32_t writeSingleCoil(const uint16_t& _slaveAddr, uint16_t _startAddr, bool value) override;

    int32_t writeSingleRegister(const uint16_t& _slaveAddr, uint16_t _startAddr, uint16_t _value) override;

    int32_t writeMultipleCoils(const uint16_t& _slaveAddr, uint16_t _startAddr, uint16_t _quantity, const uint8_t* _coils) override;

    int32_t writeMultipleRegisters(const uint16_t& _slaveAddr, uint16_t _startAddr, uint16_t _quantity, const uint16_t* _registers) override;

    int32_t readFileRecord(const uint16_t& _slaveAddr, uint16_t _fileNumber, uint16_t _recordNumber, uint16_t* _registers, uint16_t _count) override;

    int32_t writeFileRecord(const uint16_t& _slaveAddr, uint16_t _fileNumber, uint16_t _recordNumber, const uint16_t* _registers, uint16_t _count) override;

    void process() override;
private:
    ModbusMasterImpl(std::shared_ptr<ModbusComInterface>& _comIf,
                     std::shared_ptr<ModbusHandleInterface> _handle);

    bool initialized();

    nmbs_platform_conf m_nanoModbusPlatform{};
    nmbs_t m_nanoModbus{};
};


#endif //BSS_SDK_MODBUSMASTERIMPL_H
