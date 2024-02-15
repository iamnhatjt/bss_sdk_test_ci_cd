//
// Created by vnbk on 18/09/2023.
//

#ifndef BSS_SDK_MODBUSCOMIMPL_H
#define BSS_SDK_MODBUSCOMIMPL_H

#include "ModbusComInterface.h"
#include "SerialPort.h"

class ModbusComImpl : public ModbusComInterface{
public:
    static std::shared_ptr<ModbusComImpl> create(std::shared_ptr<SerialPort>& _serialPort);

    ~ModbusComImpl() override;

    int32_t open(int32_t _baudRate) override;

    int32_t close() override;

    int32_t config(int32_t _baudRate) override;

    int32_t getBaudRate() override;

    int32_t send(const void* _data, int32_t _len, int32_t _timeout) override;

    int32_t recv(void* _data, int32_t _maxLen, int32_t _timeout) override;


private:
    explicit ModbusComImpl(std::shared_ptr<SerialPort>& _serialPort);

    bool initialized();

    void process();

    std::shared_ptr<SerialPort> m_serialPort;
};

#endif //BSS_SDK_MODBUSCOMIMPL_H
