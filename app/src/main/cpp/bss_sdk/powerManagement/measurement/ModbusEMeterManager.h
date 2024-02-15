//
// Created by vnbk on 24/07/2023.
//

#ifndef BSS_SDK_MODBUSEMETERMANAGER_H
#define BSS_SDK_MODBUSEMETERMANAGER_H

#include <memory>
#include <vector>

#include "EMeterManagerInterface.h"
#include "ModbusMasterInterface.h"

#include "ModbusRtuEMeterInterface.h"
#include "Timer.h"

#define PERIOD_UPDATE_TIME                  (60*1000) ///60s
#define CHECK_EMETER_ACTIVE_TIME_RETRIED    (10)

class ModbusEMeterManager : public EMeterManagerInterface{
public:
    static std::shared_ptr<ModbusEMeterManager> create(std::shared_ptr<ModbusMasterInterface>& _modbusMaster,
                                                       int32_t _periodUpdate = PERIOD_UPDATE_TIME);

     ~ModbusEMeterManager() override;

    const std::string getManufactureActive() override;

     void addModbusRtuEMeterSupport(std::shared_ptr<ModbusRtuEMeterInterface>& _modbusEMeter);

    void setReadingPeriod(int32_t _period) override;

    EMeterInfo& getAll() override;

    float getVol() override;

     float getCur() override;

     float getCos() override;

     float getFreq() override;

     float getKwh() override;

     void process() override;

private:
    explicit ModbusEMeterManager(std::shared_ptr<ModbusMasterInterface>& _modbusMaster,
                                 int32_t _periodUpdate);

    bool initialized();

    int32_t updateValue(uint8_t _paramIndex, const uint16_t* _regValue);

    int32_t updateValue(const uint16_t& _startAddr, const uint16_t& _quantity, const uint16_t* _reg);

    int32_t readEMeterParam(const std::shared_ptr<ModbusRtuEMeterInterface>& _emeter);

    void reset();

    std::shared_ptr<ModbusMasterInterface> m_modbusMaster;
    EMeterInfo m_info;

    std::vector<std::shared_ptr<ModbusRtuEMeterInterface>> m_modbusRtuEMeterSupport;
    std::shared_ptr<ModbusRtuEMeterInterface> m_activeEMeter;

    int32_t m_periodUpdate;
    WaitTimer m_periodUpdateTimeout;

    int32_t m_retry;
};


#endif //BSS_SDK_MODBUSEMETERMANAGER_H
