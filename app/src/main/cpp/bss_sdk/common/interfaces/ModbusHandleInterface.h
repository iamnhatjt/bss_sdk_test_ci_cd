//
// Created by vnbk on 18/09/2023.
//

#ifndef BSS_SDK_MODBUSHANDLEINTERFACE_H
#define BSS_SDK_MODBUSHANDLEINTERFACE_H

#include <memory>

class ModbusHandleInterface{
public:
    virtual int32_t handleReadCoils() = 0;

    virtual int32_t handleReadDiscreteInputs() = 0;

    virtual int32_t handleReadHoldingRegisters() = 0;

    virtual int32_t handleReadInputRegisters() = 0;

    virtual int32_t handleWriteSingleCoil() = 0;

    virtual int32_t handleWriteSingleRegister() = 0;

    virtual int32_t handleWriteMultipleCoils() = 0;

    virtual int32_t handleWriteMultipleRegisters() = 0;
};

#endif //BSS_SDK_MODBUSHANDLEINTERFACE_H
