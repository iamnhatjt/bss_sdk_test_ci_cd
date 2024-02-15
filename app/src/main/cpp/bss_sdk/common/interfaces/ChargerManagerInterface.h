//
// Created by vnbk on 24/07/2023.
//

#ifndef BSS_SDK_CHARGERMANAGERINTERFACE_H
#define BSS_SDK_CHARGERMANAGERINTERFACE_H

#include <memory>
#include <vector>

#include "ChargerObserverInterface.h"
#include "ChargerComInterface.h"

#define VOL_START_CHARGING_DEFAULT              65500 //mV
#define VOL_STOP_CHARGING_DEFAULT               67200 //mV

#define TEMP_START_CHARGING_DEFAULT             60  // Degree C
#define TEMP_STOP_CHARGING_DEFAULT              65

#define TEMP_POGO_START_CHARGING_DEFAULT        60
#define TEMP_POGO_STOP_CHARGING_DEFAULT         65

#define TEMP_CELL_START_CHARGING_DEFAULT        50
#define TEMP_CELL_STOP_CHARGING_DEFAULT         55

#define CUR_CHARGING_UPPER_THRESHOLD_DEFAULT           12000
#define CUR_CHARGING_LOWER_THRESHOLD_DEFAULT           500

#define BP_SOC_CHARGING_CONDITION_DEFAULT       95

typedef struct ChargerConfig{
    int32_t m_volStartCharging          = VOL_START_CHARGING_DEFAULT;
    int32_t m_volStopCharging           = VOL_STOP_CHARGING_DEFAULT;

    int32_t m_tempStartCharging         = TEMP_START_CHARGING_DEFAULT;
    int32_t m_tempStopCharging          = TEMP_STOP_CHARGING_DEFAULT;

    int32_t m_pogoTempStartCharging     = TEMP_POGO_START_CHARGING_DEFAULT;
    int32_t m_pogoTempStopCharging      = TEMP_POGO_STOP_CHARGING_DEFAULT;

    int32_t m_cellTempStartCharging     = TEMP_CELL_START_CHARGING_DEFAULT;
    int32_t m_cellTempStopCharging      = TEMP_CELL_STOP_CHARGING_DEFAULT;

    int32_t m_curChargingUpperThreshold = CUR_CHARGING_UPPER_THRESHOLD_DEFAULT;
    int32_t m_curChargingLowerThreshold = CUR_CHARGING_LOWER_THRESHOLD_DEFAULT;

    int32_t m_socChargingCondition      = BP_SOC_CHARGING_CONDITION_DEFAULT;

    ChargerConfig(){
        m_volStartCharging = VOL_START_CHARGING_DEFAULT;
        m_volStopCharging           = VOL_STOP_CHARGING_DEFAULT;

        m_tempStartCharging         = TEMP_START_CHARGING_DEFAULT;
        m_tempStopCharging          = TEMP_STOP_CHARGING_DEFAULT;

        m_pogoTempStartCharging     = TEMP_POGO_START_CHARGING_DEFAULT;
        m_pogoTempStopCharging      = TEMP_POGO_STOP_CHARGING_DEFAULT;

        m_cellTempStartCharging     = TEMP_CELL_START_CHARGING_DEFAULT;
        m_cellTempStopCharging      = TEMP_CELL_STOP_CHARGING_DEFAULT;

        m_curChargingUpperThreshold = CUR_CHARGING_UPPER_THRESHOLD_DEFAULT;
        m_curChargingLowerThreshold = CUR_CHARGING_LOWER_THRESHOLD_DEFAULT;

        m_socChargingCondition      = BP_SOC_CHARGING_CONDITION_DEFAULT;
    }

    ChargerConfig& operator=(const ChargerConfig& _other) = default;
}ChargerConfig;

class ChargerManagerInterface{
public:
    virtual ~ChargerManagerInterface() = default;

    void setConfig(const ChargerConfig& _config){
        m_config = _config;
    }

    void addObserver(const std::shared_ptr<ChargerObserverInterface>& _observer){
        if(!_observer){
            return;
        }
        m_observer = _observer;
    }

    virtual int32_t setCabList(const uint8_t& _chargerId, const std::vector<int32_t>& _cabList) = 0;

    virtual std::vector<int32_t> getCabList(const uint8_t& _chargerId) = 0;

    virtual int32_t setChargerConfig(const uint8_t& _chargerId,  bool _isAuto, const uint32_t& _cur, const uint32_t& _vol) = 0;

    virtual int32_t enableCab(const uint32_t& _cabId, bool _enable) = 0;

    virtual int32_t charged(const uint32_t& _cabId, bool _force) = 0;

    virtual int32_t discharged(const uint32_t& _cabId, bool _force) = 0;

    virtual int32_t getChargerNumber() = 0;

    virtual int32_t getChargerVol(const uint8_t& _chargerId) = 0;

    virtual int32_t getChargerCur(const uint8_t& _chargerId) = 0;

    virtual int32_t getChargerState(const uint8_t& _chargerId) = 0;

    virtual int32_t getChargingCab(const uint8_t& _chargerId) = 0;

    virtual void process() = 0;

protected:
    std::shared_ptr<ChargerObserverInterface> m_observer;
    ChargerConfig m_config;
};

#endif //BSS_SDK_CHARGERMANAGERINTERFACE_H
