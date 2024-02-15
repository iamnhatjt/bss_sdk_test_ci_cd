//
// Created by vnbk on 13/07/2023.
//

#ifndef BSS_SDK_BSSDEVICE_H
#define BSS_SDK_BSSDEVICE_H

#include <memory>
#include <thread>
#include <atomic>

#include "BpManagerInterface.h"

#include "CabComInterface.h"
#include "CabManagerInterface.h"
#include "CabManagement.h"

#include "ChargerObserverInterface.h"
#include "ChargerManagerInterface.h"

#include "OtaServiceObserverInterface.h"
#include "OtaServiceInterface.h"

#include "EMeterManagerInterface.h"
#include "ModbusEMeterManager.h"

#define BSS_SN_DEFAULT          "BSS_00001"
#define BSS_FAC_DATE_DEFAULT    "01/01/2023"
#define BSS_MODEL_DEFAULT       "abs"
#define BSS_LOT_NUMBER_DEFAULT  "123456"
#define BSS_HW_VERSION          "v1.0.2"

class BssStateMonitor : public CabObserverInterface,
                        public ChargerObserverInterface,
                        public OtaServiceObserverInterface{
public:
    explicit BssStateMonitor(BssEventCallback* _callbacks, void* _arg = nullptr){
        m_callbacks = _callbacks;
        m_arg = _arg;
    }

    void onReady() override{
        if(m_callbacks && m_callbacks->m_onBssIsReady){
            m_callbacks->m_onBssIsReady(m_arg);
        }
    }

    void onCabConnectionChanged(uint32_t _id, uint32_t _newState) override{
        if(m_callbacks && m_callbacks->m_onChangedConnection){
            m_callbacks->m_onChangedConnection(_id, _newState, m_arg);
        }
    }

    void onCabChangedDoorStatus(uint32_t _id, uint32_t _newStt) override{
        if(m_callbacks && m_callbacks->m_onChangedDoorStt){
            m_callbacks->m_onChangedDoorStt(_id, _newStt, m_arg);
        }
    }

    void onCabChangedFanStatus(uint32_t _id, uint32_t _newStt) override{
        if(m_callbacks && m_callbacks->m_onChangedFanStt){
            m_callbacks->m_onChangedFanStt(_id, _newStt, m_arg);
        }
    }

    void onCabChangedChargerStatus(uint32_t _id, uint32_t _newStt) override{
        if(m_callbacks && m_callbacks->m_onChangedChargerStt){
            m_callbacks->m_onChangedChargerStt(_id, _newStt, m_arg);
        }
    }

    void onCabTryReadBp(uint32_t _id, int32_t _stt) override{
        if(m_callbacks && m_callbacks->m_onTryReadBp){
            m_callbacks->m_onTryReadBp(_id, _stt, m_arg);
        }
    }

    void onBpConnected(uint32_t _id, const std::string& _sn, const std::string& _version) override{
        if(m_callbacks && m_callbacks->m_onBpConnected){
            m_callbacks->m_onBpConnected(_id, _sn, _version, m_arg);
        }
    }

    void onBpDisconnected(uint32_t _id, const std::string& _sn) override{
        if(m_callbacks && m_callbacks->m_onBpDisconnected){
            m_callbacks->m_onBpDisconnected(_id, _sn, m_arg);
        }
    }

    void onDeviceIsAssignedToBp(uint32_t _id, int32_t _isSuccess, const std::string& _devSn) override{
        if(m_callbacks && m_callbacks->m_onDeviceIsAssignedToBp){
            m_callbacks->m_onDeviceIsAssignedToBp(_id, _isSuccess, _devSn, m_arg);
        }
    }

    void onCharged(const uint8_t& _id, const uint32_t& _cabId, int32_t _isSuccess) override{
        if(m_callbacks && m_callbacks->m_onChargerCharged){
            m_callbacks->m_onChargerCharged(_id, _cabId, _isSuccess, m_arg);
        }
    }

    void onDischarged(const uint8_t& _id, const uint32_t& _cabId, CHARGER_DISCHARGED_REASON _reason) override{
        if(m_callbacks && m_callbacks->m_onChargerDischarged){
            m_callbacks->m_onChargerDischarged(_id, _cabId, _reason, m_arg);
        }
    }

    void onUpgradingReady(const uint8_t &_devType, const uint32_t &_id, bool _isReady) override {
        if (m_callbacks && m_callbacks->m_onDeviceUpgradingReady){
            m_callbacks->m_onDeviceUpgradingReady(_id, _devType, _isReady, m_arg);
            return;
        }
    }

    void onUpgradingStatus(const uint8_t &_devType,
                           const uint32_t &_id,
                           const std::string &_sn,
                           const std::string &_newVersion,
                           const int32_t &_error,
                           const std::string &_reason) override {
        if (!m_callbacks || !m_callbacks->m_onBpUpgradeFw) {
            return;
        }
        switch (_devType) {
            case BSS_DEVICE_TYPE::BSS_DEV_TYPE_BP:
                m_callbacks->m_onBpUpgradeFw(_id, _sn, _newVersion, _error, _reason, m_arg);
                break;
            case BSS_DEVICE_TYPE::BSS_DEV_TYPE_SLAVE:
                m_callbacks->m_onCabUpgradeFw(_id, _newVersion, _error, _reason, m_arg);
                break;
            default:
                break;
        }
    }

    void onUpgradingFinished() override{
        if (!m_callbacks || !m_callbacks->m_onDeviceUpgradingFinish) {
            return;
        }
        m_callbacks->m_onDeviceUpgradingFinish(m_arg);
    }

public:
    BssEventCallback* m_callbacks;
    void* m_arg;
};

class BssDevice {
public:
    static std::shared_ptr<BssDevice> create(const BssDeviceConfiguration& _deviceConfig,
                                             std::shared_ptr<BssStateMonitor> _monitor);

    static SelexManufactureInfo m_manuInfoDefault;
    static BssDeviceConfiguration m_bssConfigDefault;
    static void setDeviceConfigDefault();
    static void setManufactureInfoDefault();

    ~BssDevice();

    /**
     * @brief Reboot BSS Device.
     * @return 0 - SUCCESS,
     *         -1 - FAILURE
     */
    int32_t reboot();

    const SelexManufactureInfo &getManufacture();

    int32_t setConfiguration(const BssDeviceConfiguration &_config);

    const BssDeviceConfiguration &getConfiguration();

    /**
     * @brief Get cabinet number of BSS
     * @return Number of cabinet.
     */
    int32_t getCabNumber() const;

/**
 * @brief Get Electric Meter that is measure. Ex: Current, Voltage, Energy...
 * @param _meter Electric meter value.
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
    int32_t getEMeterData(BssEMeterData &_meter);

    /**
 * @brief Get name and manufacture name of EMeter. Return '\0' in case EMeter FAILRE or could NOT communicate with EMeter.
 * @return name_manufacture - SUCCESS
 *         "\0"             - FAILURE
 */
    const char* getEMeterName();

/**
 * @brief Set the meter reading cycle
 * @param _period is reading cycle
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
    int32_t setReadingCycle(int32_t _period);


    /**
 * @brief This is function that communicate with all Slaves. They have to run into the supper loop.
 * @return  0 - SUCCESS,
 *         -1 - FAILURE
 */
    int32_t process();

/**
 * @brief Get Cabinet Information. These field is in BssCabData Struct.
 * @param _id
 * @param _CabComData
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
    const BssCabData &cabGetData(uint32_t _id);

/**
 * @brief Get Empty Cabinet List.
 * @param _list
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
    std::vector<uint32_t> cabGetEmpty();

/**
 * @brief Check cabinet is empty or BP is available.
 * @param _id is ID of cabinet.
 * @return true - if the cabinet is empty,
 *         false - other.
 */
    bool cabIsEmpty(uint32_t _id);

    /**
     * @brief Get total cabinet that have Battery.
     * @return Array ID cabinets.
     */
    std::vector<uint32_t> cabsWithBp();

    /**
 * @brief Control Fan of Cabinet. 1 - turn on, 0 - turn off.
 * @param _id is ID of cabinet
 * @param _fan_status
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
    int32_t cabCtlFan(const uint32_t &_id, int32_t _fan_status);

/**
 * @brief Get Fan status of Cabinet.
 * @param _id is ID of cabinet.
 * @return 1 - if the Fan of cabinet is turn on,
 *         0 - other.
 */
    int32_t cabGetFanStatus(const uint32_t &_id);

/**
 * @brief Control ChargerCom. 1 - charge, 0 - discharge.
 * @param _id is ID of cabinet.
 * @param _charger_state
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
    int32_t cabCtlChargerSwitch(const uint32_t &_id, const int32_t &_charger_state);

/**
 * @brief Get ChargerCom status of Cabinet.
 * @param _id is ID of cabinet.
 * @return 1 - if the ChargerCom is turn on,
 *         0 - other.
 */
    int32_t cabGetChargerSwitchStatus(const uint32_t &_id);

/**
 * @brief Get door status of cabinet.
 * @param _id is ID of cabinet.
 * @return 1 - if the Door of cabinet is open,
 *         0 - other.
 */
    int32_t cabGetDoorStatus(const uint32_t &_id);

/**
 * @brief Open door of cabinet.
 * @param _id is ID of cabinet.
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
    int32_t cabOpenDoor(const uint32_t &_id);

/**
 * @brief Get Cabinet State.
 * @param _id is ID of cabinet.
 * @return state of cabinet.
 */
    int32_t cabIsConnected(const uint32_t &_id);

/**
 * @brief Get temperatures of cabinet. temps[0]: temp of cabinet, temps[0+n]: temperature of pogo PIN (n=1)
 * @param _id is ID of cabinet.
 * @param temps is temperatures
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
    std::vector<int32_t> cabGetTemps(const uint32_t &_id);

    /**
     * @brief This method will assigned EV/BSS Serial Number to BP. The result is returned to "onDeviceIsAssignedToBp" callback
     * @param _id is ID of cabinet. From 0
     * @return  0 - SUCCESS
     *          -1 - FAILURE
     */
    int32_t cabAssignedDeviceToBp(const uint32_t& _id, const std::string&);

    /**
     * @brief This method will try assign BP and read BP information.
     * @param _id is ID of cabinet. From 0.
     * @param _count is number of tries.
     * @return 0 - SUCCESS
     *         -1 - FAILURE
     */
    int32_t cabTryReadBp(const uint32_t& _id, int32_t _count);

    /**
 * @brief Upgrade firmware for Slave of Cabinet. NOTE: This function just active with Slave V2.0.0
 * @param _id is ID of Cabinet
 * @param _newVersion is new version of Slave.
 * @param _path is file path of firmware (ex. /data/selex/bp/v1.0.14.hex)
 * @return  0 - SUCCESS
 *          -1 - FAILURE
 */
    int32_t cabUpgradeFw(const uint32_t& _id, const std::string& _newVersion, const std::string& _path);

    /**
     * @brief This method that is get BP data with the condition that Cabinet have BP.
     * @param _id is ID of cabinet. From 0.
     * @return Data of Bp.
     */
    const BpData& bpGetData(const uint32_t& _id);

    /**
 * @brief Upgrade firmware for BP.
 * @param _id is ID of Cabinet.
 * @param _sn is serial number of BP.
 * @param _newVersion is new version name.
 * @param _path is file path of firmware (ex. /data/selex/bp/v1.0.14.hex)
 * @return  0 - SUCCESS
 *          -1 - FAILURE
 */
    int32_t bpUpgradeFw(const uint32_t& _id, const std::string& _sn, const std::string& _newVersion, const std::string& _path);

/**
 * @brief Charge charger. The result is returned on onChargerCharged Callback.
 * @param _cabId is ID of Cabinet
 * @return 0 - SUCCESS
 *         1 - FAILURE
 */
    int32_t chargerCharge(const uint32_t& _cabId);

/**
 * @brief Discharge charger. The result is returned on onChargerDischarged Callback.
 * @param _cabId is ID of Cabinet
 * @return 0 - SUCCESS
 *         1 - FAILURE
 */
    int32_t chargerDischarge(const uint32_t& _cabId);

    /**
 * @brief Get charger number that is bss support.
 * @return Number of charger.
 */
    int32_t chargerGetNumber();

    /**
 * @brief Set cabinet list that charger manage.
 * @param _id is ID of Charger
 * @param _cabs are ID cabinet list.
 * @param _number is number of cabinet.
 * @return 0 - SUCCESS
 *         1 - FAILURE
 */
    int32_t chargerSetCabList(const uint8_t& _chargerId, const uint32_t* _cabs, uint32_t _number);

    /**
 * @brief Get cabinet list that charger manage.
 * @param _chargerId is ID of Charger
 * @return 0 - SUCCESS
 *         1 - FAILURE
 */
    std::vector<int32_t> chargerGetCabList(const uint8_t& _chargerId);

/**
 * @brief Get state of charger. The State is defined in bss_data_def.h file. Ex: CHARGER_STATE_DISCHARGED
 * @param _id is ID of charger.
 * @return 0 - SUCCESS
 *         1 - FAILURE
 */
    int32_t chargerGetState(const uint8_t& _chargerId);

/**
 * @brief Get current of charger.
 * @param _id is ID of charger.
 * @return The current of charger.
 */
    int32_t chargerGetCur(const uint8_t& _chargerId);

/**
 * @brief Get Voltage of charger.
 * @param _id is ID of charger.
 * @return The voltage of charger.
 */
    int32_t chargerGetVol(const uint8_t& _chargerId);

    /**
 * @brief Get cabinet ID that is charging by Charger.
 * @param _id is ID of charger.
 * @return Cabinet ID.
 *         -1: If charger is discharged at this cabinet.
 */
    int32_t chargerGetChargingCab(const uint8_t& _chargerId);

/**
 * @brief Enable cabinet that can be charged
 * @param _cabId is Cabinet ID
 * @param _enable
 * @return 0 - SUCCESS
 *         1 - FAILURE
 */
    int32_t chargerEnableCab(const uint32_t& _cabId, bool _enable);

private:
    explicit BssDevice(std::shared_ptr<BssStateMonitor> _monitor);

    bool loadManufactureInfo();
    bool initializedSystemConfig();

    bool initialized(const BssDeviceConfiguration &_deviceConfig);

    void bpServiceRun();

    std::shared_ptr<CabinetManager> m_cabManager;
    std::shared_ptr<BpManagerInterface> m_bpManager;
    std::shared_ptr<ChargerManagerInterface> m_chargerManager;
    std::shared_ptr<OtaServiceInterface> m_otaService;
    std::shared_ptr<ModbusEMeterManager> m_emeterManager;

    std::shared_ptr<BssStateMonitor> m_bssMonitor;

    SelexManufactureInfo m_manuInfo;
    BssDeviceConfiguration m_deviceConfig;

    std::thread m_bpThread;
    std::atomic_bool m_bpThreadShutdown;

};

#endif //BSS_SDK_BSSDEVICE_H
