//
// Created by vnbk on 13/07/2023.
//

#include "BssDevice.h"

#include <utility>
#include "Logger.h"

#include "CabCommunication.h"
#include "BpCoCom.h"

#include "BpManagement.h"
#include "CabManagement.h"
#include "ChargerManager.h"
#include "OtaService.h"

#include "ModbusEMeterManager.h"
#include "ModbusMasterImpl.h"
#include "ModbusComImpl.h"
#include "Vse11ModbusEMeter.h"
#include "Dds6619ModbusEMeter.h"

//#ifdef BSS_VERSION_3_1_0
#include "BssModbusCom.h"
//#endif

#define TAG "BssDevice"

BssDeviceConfiguration BssDevice::m_bssConfigDefault;

void BssDevice::setDeviceConfigDefault() {
    m_bssConfigDefault.m_chargerConfig.m_volStartCharging = VOL_START_CHARGING_DEFAULT;
    m_bssConfigDefault.m_chargerConfig.m_volStopCharging = VOL_STOP_CHARGING_DEFAULT;
    m_bssConfigDefault.m_chargerConfig.m_tempStartCharging = TEMP_START_CHARGING_DEFAULT;
    m_bssConfigDefault.m_chargerConfig.m_tempStopCharging = TEMP_STOP_CHARGING_DEFAULT;
    m_bssConfigDefault.m_chargerConfig.m_pogoTempStartCharging = TEMP_POGO_START_CHARGING_DEFAULT;
    m_bssConfigDefault.m_chargerConfig.m_pogoTempStopCharging = TEMP_POGO_STOP_CHARGING_DEFAULT;
    m_bssConfigDefault.m_chargerConfig.m_cellTempStartCharging = TEMP_CELL_START_CHARGING_DEFAULT;
    m_bssConfigDefault.m_chargerConfig.m_cellTempStopCharging = TEMP_CELL_STOP_CHARGING_DEFAULT;
    m_bssConfigDefault.m_chargerConfig.m_curChargingUpperThreshold = CUR_CHARGING_UPPER_THRESHOLD_DEFAULT;
    m_bssConfigDefault.m_chargerConfig.m_curChargingLowerThreshold= CUR_CHARGING_LOWER_THRESHOLD_DEFAULT;

    m_bssConfigDefault.m_cabNumber = BSS_CAB_NUMBER_DEFAULT;
    m_bssConfigDefault.m_cabPort = "/dev/ttyUSB0";
    m_bssConfigDefault.m_cabPortBaudRate = 115200;
}

SelexManufactureInfo BssDevice::m_manuInfoDefault;

void BssDevice::setManufactureInfoDefault() {
    m_manuInfoDefault.m_sn = BSS_SN_DEFAULT;
    m_manuInfoDefault.m_model = BSS_MODEL_DEFAULT;
    m_manuInfoDefault.m_hwVersion = BSS_HW_VERSION;
    m_manuInfoDefault.m_factoryDate = BSS_FAC_DATE_DEFAULT;
    m_manuInfoDefault.m_lotNumber = BSS_LOT_NUMBER_DEFAULT;
}

std::shared_ptr<BssDevice> BssDevice::create(const BssDeviceConfiguration& _deviceConfig,
                                             std::shared_ptr<BssStateMonitor> _monitor) {
   auto bssDevice = std::shared_ptr<BssDevice>(new BssDevice(std::move(_monitor)));
   if(bssDevice->initialized(_deviceConfig)){
       LOG_INFO(TAG, "Created Bss Device SUCCESS");
       return bssDevice;
   }
    return nullptr;
}

BssDevice::BssDevice(std::shared_ptr<BssStateMonitor> _monitor) : m_bpManager(nullptr),
                                                m_cabManager(nullptr),
                                                m_bpThreadShutdown(false){
    m_bssMonitor = std::move(_monitor);
}

int32_t BssDevice::reboot() {
    m_bpThreadShutdown = true;
    return initialized(m_deviceConfig);
}

BssDevice::~BssDevice() {
    if(m_bpThread.joinable()){
        m_bpThread.join();
    }

    m_bpManager.reset();
    m_cabManager.reset();
    m_bssMonitor.reset();
}

bool BssDevice::loadManufactureInfo() {

    return true;
}

bool BssDevice::initializedSystemConfig() {
    return true;
}

bool BssDevice::initialized(const BssDeviceConfiguration &_deviceConfig){
    m_deviceConfig = m_bssConfigDefault;
    m_deviceConfig.m_cabPortBaudRate = 115200;
    m_deviceConfig.m_cabNumber = BSS_CAB_NUMBER_DEFAULT;

    setManufactureInfoDefault();

    if(findUsbSerialConverterSupport() < 0){
        LOG_ERROR(TAG, "Could NOT find USB Serial Converter support for Cabinet interface");
        return false;
    }

    std::string cabSerialPortFile = getUsbSerialConverterSupport(0);
    LOG_INFO(TAG,"Cabinet port is port %s", cabSerialPortFile.c_str());

    cabSerialPortFile = cabSerialPortFile.empty() ? "/dev/ttyUSB0" : cabSerialPortFile;

    auto cabSerial = std::make_shared<SerialPort>(cabSerialPortFile);

#ifdef BSS_VERSION_3_0_0
    auto cabInterface = CabCommunication::create(cabSerial, CAB_SERIAL_PORT_BAUD);
    if(!cabInterface){
        LOG_ERROR(TAG, "Created Cabinet Interface FAILURE");
        return false;
    }
#endif

#ifdef BSS_VERSION_3_1_0
    std::shared_ptr<ModbusComInterface> modbusComIf = ModbusComImpl::create(cabSerial);
    if(!modbusComIf){
        LOG_ERROR(TAG, "Created Modbus Interface FAILURE");
    }

    std::shared_ptr<ModbusMasterInterface> modbusMaster = ModbusMasterImpl::create(modbusComIf);
    if(!modbusMaster){
        LOG_ERROR(TAG, "Created Modbus RTU Master FAILURE");
    }

    auto bssCom = BssModbusCom::create(modbusMaster);
    if(!bssCom){
        LOG_ERROR(TAG, "Created BSS Communication FAILURE");
    }
#endif

#ifdef BP_CO_INTERFACE
#ifdef CO_ETHERNET_INTERFACE
    sm_co_if_t* coInterface = sm_co_if_create_default(CO_ETHERNET_CANBUS_IF,
                                                      CO_TCP_SERVER_PORT,
                                                      CO_TCP_SERVER_IP,
                                                      nullptr);
#endif /// CO_ETHERNET_INTERFACE

#ifdef CO_SERIAL_PORT_INTERFACE
    LOG_INFO(TAG, "Canbus Serial Port: %s.\nNumber Cabinet: %d", CANBUS_SERIAL_PORT, BSS_CAB_NUMBER_DEFAULT);

    sm_co_if_t* coInterface = sm_co_if_create_default(CO_SERIAL_CANBUS_IF,
                                                      CANBUS_SERIAL_PORT,
                                                      CO_SERIAL_PORT_BAUD,
                                                      nullptr);
#endif /// CO_SERIAL_PORT_INTERFACE

    auto bpComInterface = BpCoCom::create(cabInterface, coInterface, m_deviceConfig.m_cabNumber);

#endif ///BP_CO_INTERFACE

#ifdef BP_MODBUS_INTERFACE
    /// TODO: Create modbus RTU interface for Battery Communication with Cabinet Communication.
#endif ///BP_MODBUS_INTERFACE

#ifdef BSS_VERSION_3_1_0
    const auto& bpComInterface = bssCom;
#endif

    auto bpManager = BpManager::create(bpComInterface, m_deviceConfig.m_cabNumber);
    if(!bpManager){
        LOG_ERROR(TAG, "Created Bp Manager FAILURE");
        return false;
    }
    bpComInterface->addManager(bpManager);


#ifdef BSS_VERSION_3_1_0
    const auto& cabInterface = bssCom;
#endif
    auto cabManager = CabinetManager::create(cabInterface,
                                             bpManager,
                                             m_deviceConfig.m_cabNumber,
                                             CAB_TEMP_OFFSET,
                                             CAB_POGO_TEMP_OFFSET);
    if(!cabManager){
        LOG_ERROR(TAG, "Created Cabinet Manager FAILURE");
        return false;
    }

    this->m_bpManager = bpManager;
    this->m_cabManager = cabManager;

    this->m_bpManager->addObserver((const std::shared_ptr<BpObserverInterface>&)(m_cabManager));
    this->m_cabManager->addObserver(m_bssMonitor);

    /**
     * Create Charger Manager that manage charger
     */


    auto chargerManager = ChargerManager::create(nullptr, m_bpManager, m_cabManager, BSS_CHARGER_NUMBER_DEFAULT);
    if(!chargerManager){
        LOG_ERROR(TAG, "Created Charger Manager FAILURE");
        return false;
    }
    chargerManager->setCabList(0, LIST_CABS_MANAGED_BY_CHARGER_0_DEFAULT);
    chargerManager->setCabList(1, LIST_CABS_MANAGED_BY_CHARGER_1_DEFAULT);

    this->m_chargerManager = chargerManager;

    m_manuInfo = m_manuInfoDefault;

    /**
     * Create OTA service to upgrade firmware to BP or Slave
     */
    auto ota = OtaService::create(m_bpManager,
                                  static_cast<const std::shared_ptr<CabManagerInterface>&>(m_cabManager),
                                  nullptr
#ifdef BSS_VERSION_3_1_0
                                  ,modbusMaster
#endif
                                  );
    if(!ota){
        LOG_ERROR(TAG, "Created OTA Service FAILURE");
        return false;
    }
    m_otaService = ota;
    m_otaService->addObserver(m_bssMonitor);

    /**
     * Create EMeter Measurement.
     */
#ifdef BSS_VERSION_3_0_0
    std::shared_ptr<ModbusComInterface> modbusComIf = ModbusComImpl::create(cabSerial);
    if(!modbusComIf){
        LOG_ERROR(TAG, "Created Modbus Interface FAILURE");
    }

    std::shared_ptr<ModbusMasterInterface> modbusMaster = ModbusMasterImpl::create(modbusComIf);
    if(!modbusMaster){
        LOG_ERROR(TAG, "Created Modbus RTU Master FAILURE");
    }
#endif

    auto emeterManager = ModbusEMeterManager::create(modbusMaster, BSS_EMETER_UPDATE_TIME_PERIOD);
    if(!emeterManager){
        LOG_ERROR(TAG, "Create Electric Meter Manager FAILURE");
    } else{
        m_emeterManager = emeterManager;

        /// Electric meter Modbus RTU VSE11
        std::shared_ptr<ModbusRtuEMeterInterface> vse11EMeter = std::make_shared<Vse11ModbusEMeter>();
        m_emeterManager->addModbusRtuEMeterSupport(vse11EMeter);

        /// Electric meter Modbus RTU DDS6619 SinoTimer
        std::shared_ptr<ModbusRtuEMeterInterface> dds6619EMeter = std::make_shared<Dds6619ModbusEMeter>();
        m_emeterManager->addModbusRtuEMeterSupport(dds6619EMeter);

        LOG_INFO(TAG, "Created and Configure Electric Meter SUCCESS");
    }

    m_manuInfo = m_manuInfoDefault;

    m_bpThread = std::thread(&BssDevice::bpServiceRun, this);
    m_bpThreadShutdown = false;

    return true;
}

const SelexManufactureInfo& BssDevice::getManufacture(){
    return m_manuInfo;
}

int32_t BssDevice::setConfiguration(const BssDeviceConfiguration& _config){
    m_deviceConfig = _config;
    return 0;
}

const BssDeviceConfiguration& BssDevice::getConfiguration(){
    return m_deviceConfig;
}

int32_t BssDevice::getCabNumber() const {
    return m_deviceConfig.m_cabNumber;
}

int32_t BssDevice::getEMeterData(BssEMeterData &_meter) {
    if(m_emeterManager){
        _meter.m_isActive = !m_emeterManager->getManufactureActive().empty();
        _meter.m_vol = m_emeterManager->getVol();
        _meter.m_cur = m_emeterManager->getCur();
        _meter.m_cos = m_emeterManager->getCos();
        _meter.m_freq = m_emeterManager->getFreq();
        _meter.m_kwh = m_emeterManager->getKwh();
        return 0;
    }else{
        _meter.m_vol = 0.0;
        _meter.m_cur = 0.0;
        _meter.m_cos = 0.0;
        _meter.m_freq = 0.0;
        _meter.m_kwh = 0.00;
        _meter.m_isActive = false;
        return -1;
    }
}

int32_t BssDevice::setReadingCycle(int32_t _period) {
    if(m_emeterManager){
        m_emeterManager->setReadingPeriod(_period);
        return 0;
    }
    return -1;
}

const char *BssDevice::getEMeterName() {
    if(m_emeterManager){
        return m_emeterManager->getManufactureActive().c_str();
    }
    return "";
}

int32_t BssDevice::process() {
    if(m_cabManager) {
        m_cabManager->process();
    }

    if(m_chargerManager){
        m_chargerManager->process();
    }

    if(m_emeterManager){
//        m_emeterManager->process();
    }
    return 0;
}

void BssDevice::bpServiceRun() {
    while (!m_bpThreadShutdown){
        if(m_bpManager){
            m_bpManager->process();
        }
        if(m_otaService){
            m_otaService->process();
        }
    }
}

const BssCabData&  BssDevice::cabGetData(uint32_t _id){
    auto cab = m_cabManager->getCab(_id);
    return cab->m_data;
}

std::vector<uint32_t> BssDevice::cabGetEmpty(){
    return m_cabManager->getEmptyCabs();
}

bool BssDevice::cabIsEmpty(uint32_t _id){
    auto cab = m_cabManager->getCab(_id);
    return cab->m_data.m_bpSn.empty();
}

std::vector<uint32_t> BssDevice::cabsWithBp() {
    return m_cabManager->getCabsWithBp();
}

int32_t BssDevice::cabCtlFan(const uint32_t & _id, int32_t _value){
    return m_cabManager->ctlFan((int32_t)_id, _value);
}

int32_t BssDevice::cabGetFanStatus(const uint32_t& _id){
    auto cab = m_cabManager->getCab(_id);
    return cab->m_data.m_fanStt;
}

int32_t BssDevice::cabCtlChargerSwitch(const uint32_t& _id, const int32_t& _value){
    return m_cabManager->ctlChargerSwitch(_id, _value);
}

int32_t BssDevice::cabGetChargerSwitchStatus(const uint32_t& _id){
    auto cab = m_cabManager->getCab(_id);
    return cab->m_data.m_chargerSwitchStt;
}

int32_t BssDevice::cabGetDoorStatus(const uint32_t& _id){
    auto cab = m_cabManager->getCab(_id);
    return cab->m_data.m_doorStt;
}

int32_t BssDevice::cabOpenDoor(const uint32_t & _id){
    return m_cabManager->openDoor(_id);
}

int32_t BssDevice::cabIsConnected(const uint32_t& _id){
    return (int32_t)m_cabManager->getCab(_id)->m_data.m_isConnected;
}

std::vector<int32_t> BssDevice::cabGetTemps(const uint32_t& _id){
    std::vector<int32_t> temps;

    auto cab = m_cabManager->getCab(_id);
    temps.push_back(cab->m_data.m_temp);

    for (int m_pogoTemp : cab->m_data.m_pogoPinTemp) {
        temps.push_back(m_pogoTemp);
    }

    return temps;
}

int32_t BssDevice::cabAssignedDeviceToBp(const uint32_t& _id, const std::string& _dev_sn){
    return m_cabManager->assignDeviceToBp(_id, _dev_sn);
}

int32_t BssDevice::cabTryReadBp(const uint32_t &_id, int32_t _count) {
    LOG_INFO(TAG,"Try to read BP info at the cabinet %d", _id);
    return m_cabManager->assignBp(_id, _count);
}

int32_t BssDevice::cabUpgradeFw(const uint32_t &_id, const std::string &_newVersion, const std::string &_path) {
#ifdef BSS_VERSION_3_0_0
    LOG_ERROR(TAG, "This version is NOT support to upgrade firmware for Slave");
    return -1;
#endif
    return m_otaService->requestUpgrade(BSS_DEVICE_TYPE::BSS_DEV_TYPE_SLAVE, _id, "", _newVersion, _path);
}

const BpData& BssDevice::bpGetData(const uint32_t& _id) {
    return m_bpManager->getData(_id);
}

int32_t BssDevice::bpUpgradeFw(const uint32_t &_id,
                               const std::string &_sn,
                               const std::string &_newVersion,
                               const std::string &_path) {
#ifdef BSS_VERSION_3_0_0
    return m_otaService->requestUpgrade(BSS_DEVICE_TYPE::BSS_DEV_TYPE_BP, _id, _sn, _newVersion, _path);
#endif
    return -1;
}

int32_t BssDevice::chargerCharge(const uint32_t &_cabId) {
    if(!m_chargerManager){
        return -1;
    }
    return m_chargerManager->charged(_cabId, true);
}

int32_t BssDevice::chargerDischarge(const uint32_t &_cabId) {
    if(!m_chargerManager){
        return -1;
    }
    return m_chargerManager->discharged(_cabId, true);
}

int32_t BssDevice::chargerGetNumber(){
    if(!m_chargerManager){
        return -1;
    }
    return m_chargerManager->getChargerNumber();
}

int32_t BssDevice::chargerSetCabList(const uint8_t &_chargerId, const uint32_t* _cabs, uint32_t _number) {
    if(!_number || !_cabs){
        return -1;
    }
    std::vector<int32_t> cabs;
    for (int index = 0; index < _number; index++) {
        cabs.push_back((int32_t)_cabs[index]);
    }

    return m_chargerManager->setCabList(_chargerId, cabs);
}

std::vector<int32_t> BssDevice::chargerGetCabList(const uint8_t &_chargerId) {
    if(!m_chargerManager){
        return std::vector<int>{};
    }
    return m_chargerManager->getCabList(_chargerId);
}

int32_t BssDevice::chargerGetState(const uint8_t &_chargerId) {
    if(!m_chargerManager){
        return -1;
    }
    return m_chargerManager->getChargerState(_chargerId);
}

int32_t BssDevice::chargerGetCur(const uint8_t &_chargerId) {
    if(!m_chargerManager){
        return -1;
    }
    return m_chargerManager->getChargerCur(_chargerId);
}

int32_t BssDevice::chargerGetVol(const uint8_t &_chargerId) {
    if(!m_chargerManager){
        return -1;
    }
    return m_chargerManager->getChargerVol(_chargerId);
}

int32_t BssDevice::chargerGetChargingCab(const uint8_t &_chargerId) {
    if(!m_chargerManager){
        return -1;
    }
    return m_chargerManager->getChargingCab(_chargerId);
}

int32_t BssDevice::chargerEnableCab(const uint32_t &_cabId, bool _enable) {
    if(!m_chargerManager){
        return -1;
    }
    return m_chargerManager->enableCab(_cabId, _enable);
}
