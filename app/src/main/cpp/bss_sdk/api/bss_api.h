//
// Created by vnbk on 15/6/223.
//

#ifndef BSS_API_H
#define BSS_API_H

#include "bss_data_def.h"

/**
 * @brief Initialized BSS. This function must be called at the start of the system.
 * @param BssDeviceConfiguration: The observer that received event notification from device.
 * @return 0 - If SUCCESS.
 *         -1 - If FAILURE.
 */
int32_t bssInit(const BssDeviceConfiguration& _config,
                BssEventCallback* _callbacks,
                void* _arg);

/**
 * @brief Configure Logger BSS SDK.
 * @param _level is log level that reference the bss_data_def.h
 * @param log_puts_fn_t function to put log
 * @return 0 - If SUCCESS.
 *         -1 - If FAILURE.
 */
int32_t bssLoggerInit(int32_t _level = BSS_LOG_LEVEL::BSS_LOG_LEVEL_DEBUG, void (*log_puts_fn_t)(const char*) = nullptr);

/**
 * @brief Get Serial Number of device;
 * @return Serial Number of device.
 */
const char* bssGetSerialNumber();

/**
 * @brief Get Manufacturer Info such as Serial Number, Model, Factory Date...
 * @return Device manufacturer Information.
 */
const SelexManufactureInfo& bssGetManufacture();

/**
 * @brief Get Bss configuration.
 * @return Device Configuration.
 */
const BssDeviceConfiguration& bssGetDeviceConfiguration();

/**
 * @brief Set Bss Configuration.
 * @param _config is the configuration of device
 * @return 0   - If SUCCESS
 *         -1  - If FAILURE
 */
int32_t bssSetDeviceConfiguration(const BssDeviceConfiguration& _config);

/**
 * @brief Get system configuration.
 * @return BssSystemConfiguration
 */
const BssSystemConfiguration& bssGetSystemConfiguration();

/**
 * @brief Set System configuration.
 * @param _systemConfig
 * @return
 */
int32_t bssSetSystemConfiguration(const BssSystemConfiguration& _systemConfig);

/**
 * @brief Get Electric Meter that is measure. Ex: Current, Voltage, Energy...
 * @param _meter Electric meter value.
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
int32_t bssGetEMeterData(BssEMeterData& _meter);

/**
 * @brief Get name and manufacture name of EMeter. Return '\0' in case EMeter FAILRE or could NOT communicate with EMeter.
 * @return name_manufacture - SUCCESS
 *         "\0"             - FAILURE
 */
const char* bssGetEMeterName();

/**
 * @brief Set the meter reading cycle
 * @param _period is reading cycle
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
int32_t bssEMeterSetReadingCycle(int32_t _period);


/**
 * @brief Command from host to sync data. Data is telemetries.
 * @return
 */
int32_t bssSyncData(BssSyncData& _syncData);

/**
 * @brief This is function that communicate with all Slaves. They have to run into the supper loop.
 * @return  0 - SUCCESS,
 *         -1 - FAILURE
 */
int32_t bssProcess();

/**
 * @brief Get Cabinet Data. These field is in BssCabData Struct.
 * @param _id
 * @param _cabData
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
int32_t cabGetData(uint32_t _id, BssCabData& _cabData);

/**
 * @brief Get Empty Cabinet List.
 * @param _list
 * @return Total cabinets
 */
int32_t cabGetEmpty(uint32_t* _list);

/**
 * @brief Check cabinet is empty or BP is available.
 * @param _id is ID of cabinet.
 * @return true - if the cabinet is empty,
 *         false - other.
 */
bool    cabIsEmpty(uint32_t _id);

/**
 * @brief Get All cabinet that have Battery.
 * @param _list
 * @return Total cabinets
 */
int32_t cabsWithBp(uint32_t* _list);

/**
 * @brief Control Fan of Cabinet. 1 - turn on, 0 - turn off.
 * @param _id is ID of cabinet
 * @param _fan_value
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
int32_t cabCtlFan(const uint32_t & _id, int32_t _fan_value);

/**
 * @brief Get Fan status of Cabinet.
 * @param _id is ID of cabinet.
 * @return 1 - if the Fan of cabinet is turn on,
 *         0 - other.
 */
int32_t cabGetFanStatus(const uint32_t& _id);

/**
 * @brief Control Charger Switch on Cabinet. 1 - charge, 0 - discharge.
 * @param _id is ID of cabinet.
 * @param _value is Charging or Discharging
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
int32_t cabCtlChargerSwitch(const uint32_t& _id, const int32_t& _value);

/**
 * @brief Get Charger switch status of Cabinet.
 * @param _id is ID of cabinet.
 * @return 1 - if the ChargerCom is turn on,
 *         0 - other.
 */
int32_t cabGetChargerSwitchStatus(const uint32_t& _id);

/**
 * @brief Get door status of cabinet.
 * @param _id is ID of cabinet.
 * @return 1 - if the Door of cabinet is open,
 *         0 - other.
 */
int32_t cabGetDoorStatus(const uint32_t& _id);

/**
 * @brief Open door of cabinet.
 * @param _id is ID of cabinet.
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
int32_t cabOpenDoor(const uint32_t & _id);

/**
 * @brief Get Cabinet State.
 * @param _id is ID of cabinet.
 * @return state of cabinet.
 */
int32_t cabIsConnected(const uint32_t& _id);

/**
 * @brief Get temperatures of cabinet. temps[0]: temp of cabinet, temps[0+n]: temperature of pogo PIN (n=1)
 * @param _id is ID of cabinet.
 * @param temps is temperatures
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
int32_t cabGetTemps(const uint32_t& _id, int32_t* temps);

/**
 * @brief Assigned Device Serial (BSS or vehicle) to BP.
 * @param _id is ID of cabinet that contained BP.
 * @return 0 - SUCCESS,
 *         -1 - FAILURE
 */
int32_t cabAssignedDeviceToBp(const uint32_t& _id, const std::string&);


/**
 * @brief Try read BP in the cabinet. The result is returned on the callback function.
 * @param _id is ID of Cabinet
 * @return   0 - SUCCESS
 *          -1 - FAILURE
 */
int32_t cabTryReadBp(const uint32_t& _id, int32_t _count = 1);

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
 * @brief Sync data BP.
 * @param _BpData is reference
 * @return 0 - SUCCESS
 *         1 - FAILURE
 */
int32_t bpSyncData(BpSyncData& _bpData);

/**
 * @brief Get data of BP
 * @param _id is ID of Cabinet that storage BP.
 * @param _bpData is bpData
 * @return 0 - SUCCESS
 *         1 - FAILURE
 */
int32_t bpGetData(const uint32_t& _id, BpData& _bpData);

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
 * @brief Set cabinet list that charger manage.
 * @param _chargerId is ID of Charger
 * @param _cabs are ID cabinet list.
 * @param _number is number of cabinet.
 * @return 0 - SUCCESS
 *         1 - FAILURE
 */
int32_t chargerSetCabList(const uint8_t& _chargerId, const uint32_t* _cabs, uint32_t _number);

/**
 * @brief Get cabinet list that charger manage.
 * @param _chargerId is ID of Charger
 * @param _cabs are ID cabinet list.
 * @return 0 - SUCCESS
 *         1 - FAILURE
 */
int32_t chargerGetCabList(const uint8_t& _chargerId, uint32_t* _cabs);

/**
 * @brief Get charger number that is bss support.
 * @return Number of charger.
 */
int32_t chargerGetNumber();

/**
 * @brief Get state of charger. The State is defined in bss_data_def.h file. Ex: CHARGER_STATE_DISCHARGED
 * @param _chargerId is ID of charger.
 * @return CHARGER_STATE
 */
int32_t chargerGetState(const uint8_t& _chargerId);

/**
 * @brief Get current of charger.
 * @param _chargerId is ID of charger.
 * @return The current of charger.
 */
int32_t chargerGetCur(const uint8_t& _chargerId);

/**
 * @brief Get Voltage of charger.
 * @param _chargerId is ID of charger.
 * @return The voltage of charger.
 */
int32_t chargerGetVol(const uint8_t& _chargerId);

/**
 * @brief Get cabinet ID that is charging by Charger.
 * @param _chargerId is ID of charger.
 * @return Cabinet ID.
 *         -1: If charger is discharged at this cabinet.
 */
int32_t chargerGetChargingCab(const uint8_t& _chargerId);

/**
 * @brief Enable cabinet that can be charged
 * @param _cabId is Cabinet ID
 * @return 0 - SUCCESS
 *         1 - FAILURE
 */
int32_t chargerEnableCab(const uint32_t& _cabId, bool _enable);

#endif //BSS_API_H
