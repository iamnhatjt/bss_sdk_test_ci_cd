//
// Created by vnbk on 23/09/2023.
//

#ifndef BSS_SDK_VSE11MODBUSEMETER_H
#define BSS_SDK_VSE11MODBUSEMETER_H

#include "ModbusRtuEMeterInterface.h"

#define VSE11_VOLTAGE_FACTOR    10
#define VSE11_CURRENT_FACTOR    100
#define VSE11_ENERGY_FACTOR     100

#define VSE11_REGISTER_ENERGY     0x30001
#define VSE11_REGISTER_VOLTAGE    0x30003
#define VSE11_REGISTER_CURRENT    0x30005

#define VSE11_VOLTAGE_UPPER           (300)
#define VSE11_VOLTAGE_LOWER           (-300)
#define VSE11_CURRENT_UPPER           (40)
#define VSE11_CURRENT_LOWER           (-40)

#define VSE11_VSEE  "vse11_vsee"

class Vse11ModbusEMeter : public ModbusRtuEMeterInterface{
public:
    explicit Vse11ModbusEMeter(int32_t _addr = MODBUS_RTU_EMETER_ADDR_DEFAULT,
                      int32_t _baud = MODBUS_RTU_EMETER_BAUD_RATE_DEFAULT) : ModbusRtuEMeterInterface(VSE11_VSEE, _addr, _baud){
        addProfile(ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_ENERGY,  VSE11_REGISTER_ENERGY);
        addProfile(ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_VOLTAGE, VSE11_REGISTER_VOLTAGE);
        addProfile(ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_CURRENT, VSE11_REGISTER_CURRENT);
        m_continuous = true;
    }

    bool readContinuous(MODBUS_FUNCTION_CODE& _code, uint16_t& _startAddress, uint16_t& _quantity) override{
        if(m_continuous){
            _code = MODBUS_FUNCTION_CODE::READ_HOLDING_REGISTERS;

            uint32_t value = getProfile(ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_ENERGY);
            _startAddress = MODBUS_RTU_DECODE_REGISTER(value);
            _quantity = 2*MODBUS_RTU_EMETER_NUMBER_INDEX;
        }
        return m_continuous;
    }

    int32_t decodeValue(int32_t _paramIndex, const uint16_t* _regValue, float& _decodeValue) override {
        uint32_t decodeValue = ((uint32_t)_regValue[0] << 16) | (uint32_t)_regValue[1];
        float value = 1.0;
        switch (_paramIndex) {
            case ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_VOLTAGE:
                _decodeValue = value*((float)decodeValue)/VSE11_VOLTAGE_FACTOR;
                if(_decodeValue > VSE11_VOLTAGE_UPPER || _decodeValue < VSE11_VOLTAGE_LOWER){
                    return -1;
                }
                return 0;
            case ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_CURRENT:
                _decodeValue = value*((float)decodeValue)/VSE11_CURRENT_FACTOR;
                if(_decodeValue > VSE11_CURRENT_UPPER || _decodeValue < VSE11_CURRENT_LOWER){
                    return -1;
                }
                return 0;
            case ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_ENERGY:
                _decodeValue = value*((float)decodeValue)/VSE11_ENERGY_FACTOR;
                return 0;
            default:
                return -1;
        }
    }
};

#endif //BSS_SDK_VSE11MODBUSEMETER_H
