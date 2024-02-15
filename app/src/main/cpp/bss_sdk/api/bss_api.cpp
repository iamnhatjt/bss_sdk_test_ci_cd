//
// Created by vnbk on 15/06/2023.
//
#include "bss_api.h"
#include "BssDevice.h"
#include "Logger.h"

static std::shared_ptr<BssDevice> g_bssDevice = nullptr;
static std::shared_ptr<BssStateMonitor> g_bssMonitor = nullptr;
static std::shared_ptr<Logger> g_logger = nullptr;

#ifdef BSS_LOG_PUTS_DEFAULT
static void log_printf(const char* _log){
    printf("%s\n", _log);
}
#endif

int32_t bssInit(const BssDeviceConfiguration& _config,
                BssEventCallback* _callbacks,
                void* _arg){
    g_bssMonitor = std::make_shared<BssStateMonitor>(_callbacks, _arg);

    g_bssDevice = BssDevice::create(_config, g_bssMonitor);
    if(!g_bssDevice){
        return -1;
    }

    return 0;
}

int32_t bssLoggerInit(int32_t _level, void (*log_puts_fn_t)(const char*)){
    if(!g_logger) {
#ifdef BSS_LOG_PUTS_DEFAULT
        g_logger = std::make_shared<Logger>((Logger::LOG_LEVEL)_level, log_printf, true);
#else
        g_logger = std::make_shared<Logger>((Logger::LOG_LEVEL)_level);
#endif
    }

    g_logger->setLogLevel((Logger::LOG_LEVEL)_level);
    g_logger->setLogPutFn(log_puts_fn_t);
    return 0;
}

const char* bssGetSerialNumber(){
    if(!g_bssDevice){
        return BssDevice::m_manuInfoDefault.m_sn.c_str();
    }
    return g_bssDevice->getManufacture().m_sn.c_str();
}

const SelexManufactureInfo& bssGetManufacture(){
    if(!g_bssDevice){
        return BssDevice::m_manuInfoDefault;
    }
    return g_bssDevice->getManufacture();
}

const BssDeviceConfiguration& bssGetDeviceConfiguration(){
    if(!g_bssDevice){
        return BssDevice::m_bssConfigDefault;
    }
    return g_bssDevice->getConfiguration();
}

int32_t bssSetDeviceConfiguration(const BssDeviceConfiguration& _config){
    if(!g_bssDevice){
        return -1;
    }
    return g_bssDevice->setConfiguration(_config);
}

/**
 * @brief Get system configuration.
 * @return BssSystemConfiguration
 */
const BssSystemConfiguration& bssGetSystemConfiguration(){

}

/**
 * @brief Set System configuration.
 * @param _systemConfig
 * @return
 */
int32_t bssSetSystemConfiguration(const BssSystemConfiguration& _systemConfig){
    return 0;
}

int32_t bssGetEMeterData(BssEMeterData& _meter){
    if(!g_bssDevice){
        return -1;
    }
    return g_bssDevice->getEMeterData(_meter);
}

const char* bssGetEMeterName(){
    if(!g_bssDevice){
        return "";
    }
    return g_bssDevice->getEMeterName();
}

int32_t bssEMeterSetReadingCycle(int32_t _period){
    if(!g_bssDevice){
        return -1;
    }
    return g_bssDevice->setReadingCycle(_period);
}

int32_t bssSyncData(BssSyncData& _syncData){
    if(!g_bssDevice){
        return -1;
    }

    for(int index = 0; index < g_bssDevice->getCabNumber(); index++){
        _syncData.m_cabs[index] = g_bssDevice->cabGetData(index);
    }

    BssEMeterData eMeter;
    g_bssDevice->getEMeterData(eMeter);

    _syncData.m_eMeter = eMeter;

    return 0;
}

int32_t bssProcess(){
    if(!g_bssDevice){
        return -1;
    }
    g_bssDevice->process();
    return 0;
}

/********************************** Cabinet API ************************************/
int32_t cabGetData(uint32_t _id, BssCabData& _cabData){
    if(!g_bssDevice){
        return -1;
    }
    _cabData = g_bssDevice->cabGetData(_id);
    return 0;
}

int32_t cabGetEmpty(uint32_t* _list){
    if(!g_bssDevice){
        return -1;
    }
    std::vector<uint32_t> cabs = g_bssDevice->cabGetEmpty();
    for(auto& item : cabs){
        *_list = item;
        _list++;
    }
    return (int32_t)cabs.size();
}

bool cabIsEmpty(uint32_t _id){
    if(!g_bssDevice){
        return -1;
    }
    return g_bssDevice->cabIsEmpty(_id);
}

int32_t cabsWithBp(uint32_t* _list){
    if(!g_bssDevice){
        return -1;
    }
    std::vector<uint32_t> cabs = g_bssDevice->cabsWithBp();
    for(auto& item : cabs){
        *_list = item;
        _list++;
    }
    return (int32_t)cabs.size();
}

int32_t cabOpenDoor(const uint32_t& _id){
    if(!g_bssDevice){
        return -1;
    }
    return g_bssDevice->cabOpenDoor(_id);
}

int32_t cabGetDoorStatus(const uint32_t& _id){
    if(!g_bssDevice){
        return -1;
    }
    return g_bssDevice->cabGetDoorStatus(_id);
}

int32_t cabCtlFan(const uint32_t& _id, int32_t  _fan_state){
    if(!g_bssDevice){
        return -1;
    }
    return g_bssDevice->cabCtlFan(_id, _fan_state);
}

int32_t cabGetFanStatus(const uint32_t& _id){
    if(!g_bssDevice){
        return -1;
    }
    return g_bssDevice->cabGetFanStatus(_id);
}

int32_t cabCtlChargerSwitch(const uint32_t& _id, const int32_t& _charger_state){
    if(!g_bssDevice){
        return -1;
    }
    return g_bssDevice->cabCtlChargerSwitch(_id, _charger_state);
}

int32_t cabGetChargerSwitchStatus(const uint32_t& _id){
    if(!g_bssDevice){
        return -1;
    }
    return g_bssDevice->cabGetChargerSwitchStatus(_id);
}

int32_t cabIsConnected(const uint32_t& _id){
    if(!g_bssDevice){
        return -1;
    }
    return g_bssDevice->cabIsConnected(_id);
}

int32_t cabGetTemps(const uint32_t&  _id, int32_t* temps){
    if(!g_bssDevice){
        return -1;
    }
    std::vector<int32_t> cabTemps = g_bssDevice->cabGetTemps(_id);
    for(auto& item: cabTemps){
        *temps = item;
        temps++;
    }
    return 0;
}

int32_t cabAssignedDeviceToBp(const uint32_t& _id, const std::string& _devSn){
    if(g_bssDevice){
        return g_bssDevice->cabAssignedDeviceToBp(_id, _devSn);
    }
    return -1;
}

int32_t cabTryReadBp(const uint32_t& _id, int32_t _count){
    if(g_bssDevice){
        return g_bssDevice->cabTryReadBp(_id, _count);
    }
    return -1;
}

int32_t cabUpgradeFw(const uint32_t& _id, const std::string& _newVersion, const std::string& _path){
    if(g_bssDevice){
        return g_bssDevice->cabUpgradeFw(_id, _newVersion, _path);
    }
    return -1;
}


/********************************** BP API ************************************/
int32_t bpSyncData(BpSyncData& _bpData){
    if(g_bssDevice){
        for (int i = 0; i < g_bssDevice->getCabNumber(); ++i) {
            _bpData.m_bp[i] = g_bssDevice->bpGetData(i);
        }
    }
    return -1;
}

int32_t bpGetData(const uint32_t& _id, BpData& _bpData){
    if(g_bssDevice){
        _bpData = g_bssDevice->bpGetData(_id);
    }
    return -1;
}

int32_t bpUpgradeFw(const uint32_t& _id, const std::string& _sn, const std::string& _newVersion, const std::string& _path){
    if(g_bssDevice){
        return g_bssDevice->bpUpgradeFw(_id, _sn, _newVersion, _path);
    }
    return -1;
}


/********************************** Charger API ************************************/
int32_t chargerCharge(const uint32_t& _cabId){
    if(g_bssDevice){
        return g_bssDevice->chargerCharge(_cabId);
    }
    return -1;
}

int32_t chargerDischarge(const uint32_t& _cabId){
    if(g_bssDevice){
        return g_bssDevice->chargerDischarge(_cabId);
    }
    return -1;
}

int32_t chargerGetNumber(){
    if(g_bssDevice){
        return g_bssDevice->chargerGetNumber();
    }
    return -1;
}

int32_t chargerSetCabList(const uint8_t& _id, const uint32_t* _cabs, uint32_t _number){
    if(g_bssDevice){
        return g_bssDevice->chargerSetCabList(_id, _cabs, _number);
    }
    return -1;
}

int32_t chargerGetCabList(const uint8_t& _chargerId, uint32_t* _cabs){
    if(g_bssDevice){
        std::vector<int32_t> cabList = g_bssDevice->chargerGetCabList(_chargerId);
        for(auto& item : cabList){
            *_cabs = item;
            _cabs++;
        }
        return (int32_t)cabList.size();
    }
    return -1;
}

int32_t chargerGetState(const uint8_t& _id){
    if(g_bssDevice){
        return g_bssDevice->chargerGetState(_id);
    }
    return -1;
}

int32_t chargerGetCur(const uint8_t& _id){
    if(g_bssDevice){
        return g_bssDevice->chargerGetCur(_id);
    }
    return -1;
}

int32_t chargerGetVol(const uint8_t& _id){
    if(g_bssDevice){
        return g_bssDevice->chargerGetVol(_id);
    }
    return -1;
}

int32_t chargerGetChargingCab(const uint8_t& _id){
    if(g_bssDevice){
        return g_bssDevice->chargerGetChargingCab(_id);
    }
    return -1;
}

int32_t chargerEnableCab(const uint32_t& _cabId, bool _enable){
    if(g_bssDevice){
        return g_bssDevice->chargerEnableCab(_cabId, _enable);
    }
    return -1;
}