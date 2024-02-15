//
// Created by vnbk on 20/09/2023.
//

#ifndef BSS_SDK_MODBUSEMETERINTERFACE_H
#define BSS_SDK_MODBUSEMETERINTERFACE_H

#include <memory>
#include <map>

#define MODBUS_RTU_EMETER_ADDR_DEFAULT              1
#define MODBUS_RTU_EMETER_BAUD_RATE_DEFAULT         9600
#define MODBUS_RTU_EMETER_RESPONSE_TIMEOUT_DEFAULT  1000

class ModbusRtuEMeterInterface{
public:
    enum {
        MODBUS_RTU_EMETER_VOLTAGE = 0,
        MODBUS_RTU_EMETER_CURRENT,
        MODBUS_RTU_EMETER_COS,
        MODBUS_RTU_EMETER_FREQ,
        MODBUS_RTU_EMETER_ENERGY,
        MODBUS_RTU_EMETER_NUMBER_INDEX
    };

    explicit ModbusRtuEMeterInterface(const char* _manufacture = "", int32_t _addr = MODBUS_RTU_EMETER_ADDR_DEFAULT,
                             int32_t _baud = MODBUS_RTU_EMETER_BAUD_RATE_DEFAULT) : m_manufacture(_manufacture),
                             m_address(_addr),
                             m_baudRate(_baud),
                             m_continuous(false),
                             m_responseTimeout(MODBUS_RTU_EMETER_RESPONSE_TIMEOUT_DEFAULT){

    }

    virtual ~ModbusRtuEMeterInterface() = default;

    int32_t config(int32_t _addr, int32_t _baud){
        m_address = _addr;
        m_baudRate = _baud;

        return 0;
    }

    const std::string& getManufacture() const{
        return m_manufacture;
    }

    int32_t getAddress() const{
        return m_address;
    }

    int32_t getBaudRate() const{
        return m_baudRate;
    }

    int32_t getResponseTimeout() const{
        return m_responseTimeout;
    }

    void addProfile(int32_t _index, uint32_t _reg){
        m_funcRegisterMap.insert(std::pair<int32_t, uint32_t>(_index, _reg));
    }

    void addProfile(int32_t _index, uint8_t _functionCode, uint16_t _register){
        m_funcRegisterMap.insert(std::pair<int32_t, uint32_t>(_index, MODBUS_RTU_ENCODE_ID(_functionCode,_register)));
    }

    uint32_t getProfile(int32_t _index){
        if(m_funcRegisterMap.find(_index) != m_funcRegisterMap.end()){
            return m_funcRegisterMap.find(_index)->second;
        }
        return 0;
    }

    int32_t getIndexByAddress(const uint16_t& _address){
        for(auto& item : m_funcRegisterMap){
            uint16_t addr = MODBUS_RTU_DECODE_REGISTER(item.second);
            if(_address == addr){
                return item.first;
            }
        }
        return -1;
    }

    virtual bool readContinuous(MODBUS_FUNCTION_CODE& _code, uint16_t& _startAddress, uint16_t& _quantity){
        return m_continuous;
    }

    virtual int32_t decodeValue(int32_t _index, const uint16_t* _regValue, float& _decodeValue) = 0;
protected:
    std::string m_manufacture;
    int32_t m_address;
    int32_t m_baudRate;
    int32_t m_responseTimeout;

    bool m_continuous;

    std::map<int32_t, uint32_t> m_funcRegisterMap;
};

static inline std::string emeterParamToString(uint8_t _param){
    switch (_param) {
        case ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_VOLTAGE:
            return "MODBUS_RTU_EMETER_VOLTAGE";
        case ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_CURRENT:
            return "MODBUS_RTU_EMETER_CURRENT";
        case ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_COS:
            return "MODBUS_RTU_EMETER_COS";
        case ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_FREQ:
            return "MODBUS_RTU_EMETER_FREQ";
        case ModbusRtuEMeterInterface::MODBUS_RTU_EMETER_ENERGY:
            return "MODBUS_RTU_EMETER_ENERGY";
        default:
            return "MODBUS_RTU_EMETER_UNKNOWN";
    }
}

#endif //BSS_SDK_MODBUSEMETERINTERFACE_H
