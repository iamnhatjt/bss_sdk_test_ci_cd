//
// Created by vnbk on 18/09/2023.
//

#ifndef BSS_SDK_MODBUSCOMINTERFACE_H
#define BSS_SDK_MODBUSCOMINTERFACE_H

#include <memory>

class ModbusComInterface{
public:
    virtual ~ModbusComInterface() = default;

    virtual int32_t open(int32_t _baudRate) = 0;

    virtual int32_t close() = 0;

    virtual int32_t config(int32_t _baudRate) = 0;

    virtual int32_t getBaudRate() = 0;

    virtual int32_t send(const void* _data, int32_t _len, int32_t _timeout) = 0;

    virtual int32_t recv(void* _data, int32_t _maxLen, int32_t _timeout) = 0;

//    virtual int32_t process() = 0;
};

#endif //BSS_SDK_MODBUSCOMINTERFACE_H
