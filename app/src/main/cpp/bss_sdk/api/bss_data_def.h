//
// Created by vnbk on 05/07/2023.
//

#ifndef BSS_SDK_BSS_DATA_DEF_H
#define BSS_SDK_BSS_DATA_DEF_H

#include <cstdint>
#include <string>

#include "bss_config.h"

#define SWITCH_IO_OFF   0
#define SWITCH_IO_ON    1

typedef enum {
    BSS_LOG_LEVEL_ERROR,
    BSS_LOG_LEVEL_WARN,
    BSS_LOG_LEVEL_INFO,
    BSS_LOG_LEVEL_DEBUG,
    BSS_LOG_LEVEL_TRACE,
    LOG_LEVEL_NUMBER
}BSS_LOG_LEVEL;

typedef enum {
    CHARGER_STATE_CHARGED,
    CHARGER_STATE_DISCHARGED,
    CHARGER_STATE_PROTECTION,
    CHARGER_STATE_FAILURE,
    CHARGER_STATE_UNKNOWN,
}CHARGER_STATE;

typedef enum {
    BSS_DEV_TYPE_MASTER = 0,
    BSS_DEV_TYPE_SLAVE,
    BSS_DEV_TYPE_BP,
    BSS_DEV_TYPE_PMM,
    BSS_DEV_TYPE_NUMBER
}BSS_DEVICE_TYPE;

typedef struct SelexManufactureInfo{
    std::string m_sn;
    std::string m_model;
    std::string m_hwVersion;
    std::string m_factoryDate;
    std::string m_lotNumber;

    SelexManufactureInfo& operator=(const SelexManufactureInfo& _other){
        this->m_sn = _other.m_sn;
        this->m_model = _other.m_model;
        this->m_factoryDate = _other.m_factoryDate;
        this->m_lotNumber = _other.m_lotNumber;
        this->m_hwVersion = _other.m_hwVersion;

        return *this;
    }
}SelexManufactureInfo;

typedef struct BssEMeterData{
    bool m_isActive;
    float m_vol;
    float m_cur;
    float m_cos;
    float m_freq;
    float m_kwh;

    BssEMeterData& operator=(const BssEMeterData& _other){
        this->m_isActive = _other.m_isActive;
        this->m_cur = _other.m_cur;
        this->m_vol = _other.m_vol;
        this->m_kwh = _other.m_kwh;
        this->m_freq = _other.m_freq;
        this->m_cos = _other.m_cos;

        return *this;
    }
}BssEMeterData;

#define BSS_CAB_POGO_PIN_TEMP_SIZE      4
#define BP_CELL_TEMP_SIZE               6
#define BP_CELL_VOL_SIZE                16

enum {
    BP_STATE_INIT = 0,
    BP_STATE_IDLE,
    BP_STATE_SOFT_START,
    BP_STATE_DISCHARGING,
    BP_STATE_CHARGING,
    BP_STATE_FAULT,
    BP_STATE_SHIP_MODE,
    BP_STATE_SYSTEM_BOOST_UP,
    BP_STATE_ID_ASSIGN_START,
    BP_STATE_ID_ASSIGN_WAIT_CONFIRM,
    BP_STATE_ID_ASSIGN_CONFIRMED,
    BP_STATE_ID_ASSIGN_WAIT_SLAVE_SELECT,
    BP_STATE_START_AUTHENTICATE,
    BP_STATE_AUTHENTICATING,
    BP_STATE_STANDBY,
    BP_STATE_SHUTDOWN,
    BP_STATE_ONLY_DISCHARGING
};

typedef struct BpData{
    std::string m_sn;
    int32_t m_vol;
    int32_t m_cur;
    int32_t m_state;
    int32_t m_status;
    int32_t m_soc;
    int32_t m_soh;
    int32_t m_cycle;
    uint8_t m_temps[BP_CELL_TEMP_SIZE];
    uint16_t m_cellVols[BP_CELL_VOL_SIZE];

    std::string m_version;
    std::string m_assignedSn;

    BpData& operator=(const BpData& _other){
        this->m_sn = _other.m_sn;
        this->m_vol = _other.m_vol;
        this->m_cur = _other.m_cur;
        this->m_state = _other.m_state;
        this->m_status = _other.m_status;
        this->m_soc = _other.m_soc;
        this->m_soh = _other.m_soh;
        this->m_cycle = _other.m_cycle;
        this->m_version = _other.m_version;
        this->m_assignedSn = _other.m_assignedSn;
        for (int index = 0; index < BP_CELL_VOL_SIZE; index++) {
            this->m_cellVols[index] = _other.m_cellVols[index];
        }
        for (int index = 0; index < BP_CELL_TEMP_SIZE; index++) {
            this->m_temps[index] = _other.m_temps[index];
        }
        return *this;
    }
}BpData;

#define CAB_DOOR_OPEN_STATUS    SWITCH_IO_ON
#define CAB_DOOR_CLOSE_STATUS   SWITCH_IO_OFF

#define CAB_FAN_TURN_ON_STATUS  CAB_DOOR_OPEN_STATUS
#define CAB_FAN_TURN_OFF_STATUS CAB_DOOR_CLOSE_STATUS

#define CAB_CHARGER_TURN_ON_STATUS  CAB_FAN_TURN_ON_STATUS
#define CAB_CHARGER_TURN_OFF_STATUS CAB_FAN_TURN_OFF_STATUS

typedef struct BssCabData{
    uint8_t m_id;

    int32_t m_temp;
    int32_t m_pogoPinTemp[BSS_CAB_POGO_PIN_TEMP_SIZE];

    uint8_t m_doorStt;
    uint8_t m_fanStt;
    uint8_t m_chargerSwitchStt;

    bool m_isConnected;
    bool m_isCharged;
    bool m_isUpgrading;

    std::string m_bpSn;
    uint8_t m_bpSoc;

    BssCabData& operator=(const BssCabData& _other){
        this->m_id = _other.m_id;
        this->m_temp = _other.m_temp;
        this->m_pogoPinTemp[0] = _other.m_pogoPinTemp[0];
        this->m_pogoPinTemp[1] = _other.m_pogoPinTemp[1];
        this->m_pogoPinTemp[2] = _other.m_pogoPinTemp[2];
        this->m_pogoPinTemp[3] = _other.m_pogoPinTemp[3];
        this->m_doorStt = _other.m_doorStt;
        this->m_fanStt = _other.m_fanStt;
        this->m_chargerSwitchStt = _other.m_chargerSwitchStt;
        this->m_isConnected = _other.m_isConnected;
        this->m_isCharged = _other.m_isCharged;
        this->m_isUpgrading = _other.m_isUpgrading;
        this->m_bpSn = _other.m_bpSn;
        this->m_bpSoc = _other.m_bpSoc;

        return *this;
    }
}BssCabData;

typedef struct BssChargerData{
    uint8_t m_id;
    int32_t m_cabId;
    uint8_t m_state;
    int32_t m_vol;
    int32_t m_cur;

    BssChargerData& operator=(const BssChargerData& _other) {
        this->m_id = _other.m_id;
        this->m_cabId = _other.m_cabId;
        this->m_state = _other.m_state;
        this->m_vol = _other.m_vol;
        this->m_cur = _other.m_cur;

        return *this;
    }
}BssChargerData;

typedef struct BssSyncData{
    BssCabData m_cabs[BSS_CAB_NUMBER_DEFAULT];
    BssEMeterData m_eMeter{};
    BssChargerData m_chargers[BSS_CHARGER_NUMBER_DEFAULT]{};

    BssSyncData& operator=(const BssSyncData& _other){
        for(int index = 0; index < BSS_CAB_NUMBER_DEFAULT; index++){
            this->m_cabs[index] = _other.m_cabs[index];
        }
        this->m_eMeter = _other.m_eMeter;
        for(int index = 0; index < BSS_CHARGER_NUMBER_DEFAULT; index++){
            this->m_chargers[index] = _other.m_chargers[index];
        }
        return *this;
    }
}BssSyncData;

typedef struct BpSyncData{
    BpData m_bp[BSS_CAB_NUMBER_DEFAULT];

    BpSyncData& operator=(const BpSyncData& _other){
        for (int i = 0; i < BSS_CAB_NUMBER_DEFAULT; ++i) {
            this->m_bp[i] = _other.m_bp[i];
        }
        return *this;
    }
}BpSyncData;

typedef struct BssDeviceConfiguration{
    struct {
        int32_t m_volStartCharging;
        int32_t m_volStopCharging;

        int32_t m_tempStartCharging;
        int32_t m_tempStopCharging;

        int32_t m_pogoTempStartCharging;
        int32_t m_pogoTempStopCharging;

        int32_t m_cellTempStartCharging;
        int32_t m_cellTempStopCharging;

        int32_t m_curChargingUpperThreshold;
        int32_t m_curChargingLowerThreshold;
    }m_chargerConfig;

    uint8_t m_cabNumber;
    const char* m_cabPort;
    int32_t m_cabPortBaudRate;

    BssDeviceConfiguration& operator=(const BssDeviceConfiguration& _other) {
        this->m_chargerConfig.m_volStartCharging = _other.m_chargerConfig.m_volStartCharging;
        this->m_chargerConfig.m_volStopCharging = _other.m_chargerConfig.m_volStopCharging;
        this->m_chargerConfig.m_tempStartCharging = _other.m_chargerConfig.m_tempStartCharging;
        this->m_chargerConfig.m_tempStopCharging = _other.m_chargerConfig.m_tempStopCharging;
        this->m_chargerConfig.m_pogoTempStartCharging = _other.m_chargerConfig.m_pogoTempStartCharging;
        this->m_chargerConfig.m_pogoTempStopCharging = _other.m_chargerConfig.m_pogoTempStopCharging;
        this->m_chargerConfig.m_cellTempStartCharging = _other.m_chargerConfig.m_cellTempStartCharging;
        this->m_chargerConfig.m_cellTempStopCharging = _other.m_chargerConfig.m_cellTempStopCharging;
        this->m_chargerConfig.m_curChargingUpperThreshold = _other.m_chargerConfig.m_curChargingUpperThreshold;
        this->m_chargerConfig.m_curChargingLowerThreshold = _other.m_chargerConfig.m_curChargingLowerThreshold;

        this->m_cabNumber = _other.m_cabNumber;
        m_cabPort = _other.m_cabPort;
        m_cabPortBaudRate = _other.m_cabPortBaudRate;

        return *this;
    }
}BssDeviceConfiguration;

typedef struct BssSystemConfiguration{
    const char* m_manuFilePath;
    const char* m_usbVendorIdSupport[10];
}BssSystemConfiguration;

#define BSS_RET_SUCCESS 1
#define BSS_RET_FAILURE 0

#define CAB_STATE_CONNECTED     1
#define CAB_STATE_DISCONNECTED  0

typedef void (*onBssIsReady)(void*);
typedef void (*onCabDoorChangedStatus)(uint32_t, uint8_t, void*);
typedef void (*onCabFanChangedStatus)(uint32_t, uint8_t, void*);
typedef void (*onCabChargerSwitchChangedStatus)(uint32_t, uint8_t, void*);
typedef void (*onCabConnectionChanged)(uint32_t, uint8_t, void*);
typedef void (*onCabTryReadBp)(uint32_t, uint8_t, void*);
typedef void (*onCabUpgradeFw)(uint32_t, const std::string&, int32_t, const std::string&, void*);
typedef void (*onBpConnected)(uint32_t, const std::string&, const std::string&, void*);
typedef void (*onBpDisconnected)(uint32_t, const std::string&, void*);
typedef void (*onBpUpgradeFw)(uint32_t, const std::string&, const std::string&, int32_t, const std::string&, void*);
typedef void (*onDeviceUpgradingReady)(uint32_t, uint8_t, bool, void*);
typedef void (*onDeviceUpgradingFinished)(void*);
typedef void (*onDeviceIsAssignedToBp)(uint32_t, uint8_t, const std::string&, void*);
typedef void (*onBssSyncData)(BssSyncData&, void*);
typedef void (*onChargerCharged)(const uint8_t&, const uint32_t&, int32_t, void*);
typedef void (*onChargerDischarged)(const uint8_t&, const uint32_t&, int32_t, void*);

typedef struct BssEventCallback{
    onBssIsReady m_onBssIsReady;
    onBssSyncData m_onBssSyncData;
    onCabDoorChangedStatus m_onChangedDoorStt;
    onCabFanChangedStatus m_onChangedFanStt;
    onCabChargerSwitchChangedStatus m_onChangedChargerStt;
    onCabConnectionChanged m_onChangedConnection;
    onCabTryReadBp m_onTryReadBp;
    onCabUpgradeFw m_onCabUpgradeFw;
    onBpConnected m_onBpConnected;
    onBpDisconnected m_onBpDisconnected;
    onBpUpgradeFw m_onBpUpgradeFw;
    onDeviceUpgradingFinished m_onDeviceUpgradingFinish;
    onDeviceIsAssignedToBp m_onDeviceIsAssignedToBp;
    onDeviceUpgradingReady m_onDeviceUpgradingReady;
    onChargerCharged m_onChargerCharged;
    onChargerDischarged m_onChargerDischarged;
}BssEventCallback;

#endif //BSS_SDK_BSS_DATA_DEF_H
