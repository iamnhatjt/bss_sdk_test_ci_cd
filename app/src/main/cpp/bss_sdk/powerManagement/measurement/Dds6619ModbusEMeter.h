//
// Created by vnbk on 23/09/2023.
//

#ifndef BSS_SDK_DDS6619MODBUSEMETER_H
#define BSS_SDK_DDS6619MODBUSEMETER_H

#include "ModbusRtuEMeterInterface.h"

#define DDS6619_SINOTIMER   "dds6619_sinotimer"

#define DDS6619_REGISTER_VOLTAGE    0x40001
#define DDS6619_REGISTER_CURRENT    0x40009
#define DDS6619_REGISTER_COS        0x4002B
#define DDS6619_REGISTER_FREQ       0x40037
#define DDS6619_REGISTER_ENERGY     0x40101

#define DDS6619_VOLTAGE_UPPER           (300)
#define DDS6619_VOLTAGE_LOWER           (-300)
#define DDS6619_CURRENT_UPPER           (50)
#define DDS6619_CURRENT_LOWER           (-50)

class Dds6619ModbusEMeter : public ModbusRtuEMeterInterface{
public:
    explicit Dds6619ModbusEMeter(int32_t _addr = MODBUS_RTU_EMETER_ADDR_DEFAULT,
            int32_t _baud = MODBUS_RTU_EMETER_BAUD_RATE_DEFAULT) : ModbusRtuEMeterInterface(DDS6619_SINOTIMER, _addr, _baud){
        addProfile(ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_VOLTAGE, DDS6619_REGISTER_VOLTAGE);
        addProfile(ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_CURRENT, DDS6619_REGISTER_CURRENT);
        addProfile(ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_COS,     DDS6619_REGISTER_COS);
        addProfile(ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_FREQ,    DDS6619_REGISTER_FREQ);
        addProfile(ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_ENERGY,  DDS6619_REGISTER_ENERGY);
    }

    int32_t decodeValue(int32_t _paramIndex, const uint16_t* _regValue, float& _decodeValue) override{
        uint32_t decodeValue = ((uint32_t)_regValue[0] << 16) | (uint32_t)_regValue[1];
        union {
            uint32_t b;
            float f;
        }value{};

        value.b = decodeValue;

        _decodeValue = value.f;
        switch (_paramIndex) {
            case ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_VOLTAGE:
                if(_decodeValue > DDS6619_VOLTAGE_UPPER || _decodeValue < DDS6619_VOLTAGE_LOWER){
                    return -1;
                }
                break;
            case ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_CURRENT:
                if(_decodeValue > DDS6619_CURRENT_UPPER || _decodeValue < DDS6619_CURRENT_LOWER){
                    return -1;
                }
                break;
            default:
                break;
        }
        return 0;
    }
};

#endif //BSS_SDK_DDS6619MODBUSEMETER_H
