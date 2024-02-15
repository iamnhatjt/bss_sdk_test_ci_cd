//
// Created by vnbk on 21/09/2023.
//
#include "Logger.h"
#include "bss_api.h"

#include <thread>

#define MAIN_TAG "BP_TESTING"

#define BP_FIRMWARE_0   "/home/pi/Documents/bms_v1.0.13.rc01.hex"
#define BP_FIRMWARE_1   "/home/pi/Documents/bms_v1.0.13.rc02.hex"
#define BP_FIRMWARE_2   "/home/pi/Documents/bms_v1.0.13.rc03.hex"
#define BP_FIRMWARE_3   "/home/pi/Documents/bms_v1.0.13.rc04.hex"

#define BP_NUMBER   4

static bool         g_waitingUpgradeFinish = false;
static const char*  g_bpFirmware[] = {BP_FIRMWARE_0, BP_FIRMWARE_1, BP_FIRMWARE_2, BP_FIRMWARE_3};
static int32_t      g_curVersion = 0;

typedef struct {
    int32_t m_total;
    int32_t m_success;
    int32_t m_fail;
}bpTestingResult;

static bpTestingResult g_bpsTestingResult[BP_NUMBER];

static BssDeviceConfiguration g_bssConfigDefault;

void test_onReady(void* _arg){
    LOG_INFO(MAIN_TAG, "Bss is Ready");
}

void test_onCabDoorChangedStatus(uint32_t _id, uint8_t _isSuccess, void* _arg){
    if(_isSuccess){
        LOG_INFO(MAIN_TAG, "Cabinet Door %d changed status to %d", _id,  _isSuccess);
    }else{
        LOG_WARN(MAIN_TAG, "Cabinet Door %d changed status to %d", _id,  _isSuccess);
    }
}

void test_onCabFanChangedStatus(uint32_t _id, uint8_t _isSuccess, void* _arg){
    LOG_WARN(MAIN_TAG, "Cabinet Fan %d changed status to %d", _id,  _isSuccess);
}
void test_onCabChargerChangedStatus(uint32_t _id, uint8_t _isSuccess, void* _arg){
    LOG_WARN(MAIN_TAG, "Cabinet ChargerCom %d changed status to %d", _id,  _isSuccess);
}
void test_onCabConnectionChanged(uint32_t _id, uint8_t _isSuccess, void* _arg){
//    LOG_WARN(MAIN_TAG, "Cabinet State %d changed status to %s", _id,  _isSuccess ? "CONNECTED" : "DISCONNECTED");
}
void test_onCabTryReadBp(uint32_t _id, uint8_t _isSuccess, void* _arg){
//    LOG_WARN(MAIN_TAG, "Cabinet try read BP %d: ", _id,  _isSuccess);
}
void test_onBpConnected(uint32_t _id, const std::string& _sn, const std::string& _version, void* _arg){
//    LOG_WARN(MAIN_TAG, "BP %d is connected: SN: %s", _id,  _sn.c_str());

    BssCabData cabData;
    cabGetData(_id, cabData);

    LOG_INFO(MAIN_TAG, "BP %d: %s is connected, SOC: %d", _id, cabData.m_bpSn.c_str(), cabData.m_bpSoc);
}
void test_onBpDisconnected(uint32_t _id, const std::string& _sn, void* _arg){
    LOG_WARN(MAIN_TAG, "BP %d is disconnected: SN: %s", _id,  _sn.c_str());
}
void test_onDeviceIsAssignedToBp(uint32_t _id, uint8_t _success, const std::string& _devSn, void* _arg){
    LOG_WARN(MAIN_TAG, "Device %s is assigned to BP %d", _devSn.c_str(),  _id);
}
void test_onBssSyncData(BssSyncData& _syncData, void* _arg){
}

void test_onBpUpgradeFw(uint32_t _id, const std::string& _sn, const std::string& _newVersion, int32_t _err, const std::string& _errString, void* _arg){
//    LOG_WARN(MAIN_TAG, "Bp %d : %s is upgraded %s with new version: %s", _id, _sn.c_str(), _errString.c_str(), _newVersion.c_str());
    if(!_err){
        g_bpsTestingResult[_id].m_success++;
    }else{
        g_bpsTestingResult[_id].m_fail++;
        LOG_ERROR(MAIN_TAG, "Bp %d upgrade FAILURE, reason: %s", _id, _errString.c_str());
    }
    LOG_INFO(MAIN_TAG, "Bp %d: %s is upgrading info: total: %d, success: %d, fail: %d", _id, _sn.c_str(),
             g_bpsTestingResult[_id].m_total,
             g_bpsTestingResult[_id].m_success,
             g_bpsTestingResult[_id].m_fail);
}

void test_onUpgradingFinished(void* _arg){
    (void*)_arg;
    g_waitingUpgradeFinish = false;

    g_curVersion++;
    if(g_curVersion >= 4){
        LOG_WARN(MAIN_TAG, "Back to the first version to upgrade bp testing");
        g_curVersion = 0;
    }

    LOG_WARN(MAIN_TAG, "Upgrading progress FINISHED, switch to upgrade new version: %s", g_bpFirmware[g_curVersion]);
    LOG_INFO(MAIN_TAG, "/**********************************************************************/");
    LOG_INFO(MAIN_TAG, "\n");
}

BssEventCallback g_eventCallback;

void bss_process() {
    while (true){
        bssProcess();
    }
}

bool checkVersion(){
    for(int index = 0; index < BP_NUMBER; index++){
        BpData bpData;
        bpGetData(index, bpData);
        if(bpData.m_version.empty()){
            return false;
        }
    }
    return true;
}

int main() {
    g_eventCallback.m_onBssIsReady = test_onReady;
    g_eventCallback.m_onChangedDoorStt = test_onCabDoorChangedStatus;
    g_eventCallback.m_onChangedFanStt = test_onCabFanChangedStatus;
    g_eventCallback.m_onChangedChargerStt = test_onCabChargerChangedStatus;
    g_eventCallback.m_onChangedConnection = test_onCabConnectionChanged;
    g_eventCallback.m_onTryReadBp = test_onCabTryReadBp;
    g_eventCallback.m_onBpConnected = test_onBpConnected;
    g_eventCallback.m_onBpDisconnected = test_onBpDisconnected;
    g_eventCallback.m_onBpUpgradeFw = test_onBpUpgradeFw;
    g_eventCallback.m_onDeviceUpgradingFinish = test_onUpgradingFinished;
    g_eventCallback.m_onDeviceIsAssignedToBp = test_onDeviceIsAssignedToBp;
    g_eventCallback.m_onBssSyncData = test_onBssSyncData;

    for(auto & index : g_bpsTestingResult){
        index.m_fail = 0;
        index.m_success = 0;
        index.m_total = 0;
    }

    bssLoggerInit(BSS_LOG_LEVEL::BSS_LOG_LEVEL_DEBUG, nullptr);
    bssInit(g_bssConfigDefault, &g_eventCallback, nullptr);

    int32_t cabNumber = bssGetDeviceConfiguration().m_cabNumber;
    LOG_DEBUG(MAIN_TAG, "Cabinet Number of BSS: %d", cabNumber);

    std::thread bss = std::thread(&bss_process);

    LOG_INFO(MAIN_TAG, "/******************** START UPGRADING BP TESTING *******************/");
    LOG_INFO(MAIN_TAG, "Number cabinet for testing: 4 Cabinet");

    LOG_INFO(MAIN_TAG, "Please Enter character for start testing !!!");


    while (true){
        if(!g_waitingUpgradeFinish){
            uint32_t list[19];
            if(cabsWithBp(list) < BP_NUMBER){
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
            if(!checkVersion()){
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            LOG_INFO(MAIN_TAG, "\n");
            LOG_INFO(MAIN_TAG, "/**********************************************************************/");
            LOG_INFO(MAIN_TAG, "Starting Upgrade firmware cycle, version: %s", g_bpFirmware[g_curVersion]);

            BssCabData data;
            for(int index = 0; index < BP_NUMBER; index++){
                cabGetData(index, data);
                bpUpgradeFw(index, data.m_bpSn, "1.0.14", g_bpFirmware[g_curVersion]);

                g_bpsTestingResult[index].m_total++;
            }

            g_waitingUpgradeFinish = true;
        }
    }

    LOG_INFO(MAIN_TAG, "/******************* END UPGRADING BP TESTING *******************/");
    LOG_INFO(MAIN_TAG, "\n\n");
}
