//
// Created by vnbk on 05/09/2023.
//

#ifndef BSS_SDK_BSSMODBUSCOM_H
#define BSS_SDK_BSSMODBUSCOM_H

#include <mutex>
#include <queue>
#include <condition_variable>

#include "CabComInterface.h"
#include "BpComInterface.h"
#include "ModbusMasterInterface.h"

#include "BssModbusDefine.h"

#define BSS_MODBUS_COM_BAUD_RATE_DEFAULT    115200
#define SLAVE_ID_OFFSET                     1

class BssModbusCom : public CabComInterface,
                     public BpComInterface{
    
public:
    static std::shared_ptr<BssModbusCom> create(std::shared_ptr<ModbusMasterInterface>& _modbusMaster,
                                                std::shared_ptr<BpManagerInterface> _bpManager = nullptr,
                                                int32_t _baudRate = BSS_MODBUS_COM_BAUD_RATE_DEFAULT);

    /// Cabinet communication
    int32_t openDoor(uint32_t _id) override;

    int32_t getDoorStatus(uint32_t _id) override;

    int32_t ctlFan(uint32_t _id, uint32_t _value) override;

    int32_t getFanStatus(uint32_t _id) override;

    int32_t ctlCharger(uint32_t _id, uint32_t _value) override;

    int32_t getChargerStatus(uint32_t _id) override;

    int32_t getTemp(uint32_t _id) override;

    std::vector<int32_t> getPogoPinTemp(uint32_t _id) override;

    int32_t getAll(uint32_t _id, CabComData& _info) override;

    /// Bp communication
    void addManager(std::shared_ptr<BpManagerInterface> _manager) override;

    bool sync(bool _enable) override;

    int32_t assign(const std::shared_ptr<Bp>& _bp) override;

    int32_t setCmd(const std::shared_ptr<Bp::BpCmd>& _cmd) override;

    int32_t resetCmd() override;

    bool isBusy() override;

    int32_t process() override;

private:
    explicit BssModbusCom(std::shared_ptr<ModbusMasterInterface>& _modbusMaster,
                          std::shared_ptr<BpManagerInterface> _bpManager,
                          int32_t _baudRate);

    bool initialized();

    void decodeBpValue(const std::shared_ptr<Bp>& _bp, const std::vector<uint16_t >& _regValue);

    std::string decodeBpStringValue(const std::vector<uint16_t >& _regValue);

    uint32_t m_idOffset;

    std::shared_ptr<ModbusMasterInterface> m_modbusMaster;
    int32_t m_baudRate;

    uint16_t m_inputRegister[BSS_MODBUS_INPUT_REG_NUMBER]{};
    uint16_t m_holdingRegister[BSS_MODBUS_HOLDING_REG_NUMBER]{};

    std::shared_ptr<BpManagerInterface> m_bpManager;

    typedef struct ModbusBpData{
        uint32_t m_id;
        bool m_new;
        std::vector<uint16_t> m_sn;
        std::vector<uint16_t> m_data;

        ModbusBpData(const uint32_t &_id,
                     std::vector<uint16_t> _data,
                     bool _new = false) : m_id(_id),
                                          m_data(std::move(_data)),
                                          m_new(_new) {
        }
    }ModbusBpData;

    std::queue<std::shared_ptr<ModbusBpData>> m_bpsUpdatedData;
    std::mutex m_bpLockQueue;
    std::condition_variable m_trigger;

    std::mutex m_lock;
};


#endif //BSS_SDK_BSSMODBUSCOM_H
