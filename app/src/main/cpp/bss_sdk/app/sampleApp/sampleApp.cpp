#include <iostream>
#include "Logger.h"
#include "linux_serial.h"

#include "bss_api.h"
#include "BssDevice.h"

#include "BpCoCom.h"
#include "CabCommunication.h"

#include <thread>

#define MAIN_TAG "main"

class BpObserverTest : public BpObserverInterface{
public:
    void onBpConnected(const uint32_t& _id, const std::string& _sn, const std::string& _version) override{
        LOG_INFO(MAIN_TAG, "Bp is Connected: node id: %d, SN: %s, Version: %s", _id, _sn.c_str(), _version.c_str());
    }

    void onBpDisconnected(const uint32_t& _id, const std::string& _sn) override{
        LOG_WARN(MAIN_TAG, "Bp is disconnected: node id: %d, SN: %s", _id, _sn.c_str());
    }

    void onBpAssignStatus(const uint32_t& _id, int32_t _status) override{
        LOG_DEBUG(MAIN_TAG, "Assigning Bp status : node id: %d, status: %d", _id, _status);
    }

    void onDeviceIsAssigned(const uint32_t& _id, int32_t _isSuccess, const std::string& _bp_sn, const std::string& _dev_type, const std::string& _dev_sn) override{
//        LOG_DEBUG(MAIN_TAG, "Bp is Connected: node id: %d, SN: %s, Version: %s", _id, _sn.c_str(), _dev_sn.c_str());
    }
};

static BssDeviceConfiguration g_bssConfigDefault;/* = {
        .m_tempChargeThreshold = 45,
        .m_tempDischargeThreshold = 55,
        .m_tempReturnBpThreshold = 50,
        .m_chargingVolThreshold = 65,
        .m_dischargingVolThreshold = 67,
        .m_operatingTime = "05:00:00-22:00:00"
};*/

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
    LOG_WARN(MAIN_TAG, "Cabinet State %d changed status to %s", _id,  _isSuccess ? "CONNECTED" : "DISCONNECTED");
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
    LOG_WARN(MAIN_TAG, "Bp %d : %s is upgraded %s with new version: %s", _id, _sn.c_str(), _errString.c_str(), _newVersion.c_str());
}

void test_onUpgradingFinished(void* _arg){
    (void*)_arg;
    LOG_WARN(MAIN_TAG, "Upgrading progress FINISHED");
}

void test_printBssSyncData(BssSyncData& _data){
    LOG_INFO(MAIN_TAG, "Electric Meter: %f: %f, %f", _data.m_eMeter.m_vol,
             _data.m_eMeter.m_cur,
             _data.m_eMeter.m_kwh);

    for (int i = 0; i < BSS_CAB_NUMBER_DEFAULT; i++) {
        if(!_data.m_cabs[i].m_isConnected){
            continue;
        }
        LOG_INFO(MAIN_TAG, "Cabinet %d info: Door STT: %d, FanSTT: %d, Charger STT: %d, Temp: %d, Pogo Temps: %d,%d,%d,%d", i,
                 _data.m_cabs[i].m_doorStt,
                 _data.m_cabs[i].m_fanStt,
                 _data.m_cabs[i].m_chargerSwitchStt,
                 _data.m_cabs[i].m_temp,
                 _data.m_cabs[i].m_pogoPinTemp[0],
                 _data.m_cabs[i].m_pogoPinTemp[1],
                 _data.m_cabs[i].m_pogoPinTemp[2],
                 _data.m_cabs[i].m_pogoPinTemp[3]);
        if(_data.m_cabs[i].m_bpSn.empty()){
            continue;
        }
        LOG_INFO(MAIN_TAG, "Cabinet %d have BP with SN: %s, SOC: %d", i, _data.m_cabs[i].m_bpSn.c_str(), _data.m_cabs[i].m_bpSoc);
    }
}

void test_printBpSyncData(BpSyncData& _data){
    for (int i = 0; i < BSS_CAB_NUMBER_DEFAULT; i++) {
        if(_data.m_bp[i].m_sn.empty()){
            continue;
        }
        LOG_INFO(MAIN_TAG, "BP %d serial: %s, SOC:%d, SOH:%d, Cur:%d, Vol:%d, Cell:[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d], "
                           "Temp:[%d,%d,%d,%d,%d,%d,%d], Status:%d, State:%d", i,
                 _data.m_bp[i].m_sn.c_str(),
                 _data.m_bp[i].m_soc,
                 _data.m_bp[i].m_soh,
                 _data.m_bp[i].m_cur,
                 _data.m_bp[i].m_vol,
                 _data.m_bp[i].m_cellVols[0],
                 _data.m_bp[i].m_cellVols[1],
                 _data.m_bp[i].m_cellVols[2],
                 _data.m_bp[i].m_cellVols[3],
                 _data.m_bp[i].m_cellVols[4],
                 _data.m_bp[i].m_cellVols[5],
                 _data.m_bp[i].m_cellVols[6],
                 _data.m_bp[i].m_cellVols[7],
                 _data.m_bp[i].m_cellVols[8],
                 _data.m_bp[i].m_cellVols[9],
                 _data.m_bp[i].m_cellVols[10],
                 _data.m_bp[i].m_cellVols[11],
                 _data.m_bp[i].m_cellVols[12],
                 _data.m_bp[i].m_cellVols[13],
                 _data.m_bp[i].m_cellVols[14],
                 _data.m_bp[i].m_cellVols[15],
                 _data.m_bp[i].m_temps[0],
                 _data.m_bp[i].m_temps[1],
                 _data.m_bp[i].m_temps[2],
                 _data.m_bp[i].m_temps[3],
                 _data.m_bp[i].m_temps[4],
                 _data.m_bp[i].m_temps[5],
                 _data.m_bp[i].m_temps[6],

                 _data.m_bp[i].m_status,
                 _data.m_bp[i].m_state);
    }
}

BssEventCallback g_eventCallback;

void bss_process() {
    while (true){
        bssProcess();
    }
}

void test_bss(){
    LOG_INFO(MAIN_TAG, "/******************** TEST BSS ***********************/");
    LOG_INFO(MAIN_TAG, "Please choose the character for BSS testing!!!!");
    LOG_INFO(MAIN_TAG, "Choose '0' for get serial number cabinet!!!");
    LOG_INFO(MAIN_TAG, "Choose '1' for get Electric meter info!!!");
    LOG_INFO(MAIN_TAG, "Choose '2' for get number of cabinet!!!");
    LOG_INFO(MAIN_TAG, "Choose '3' for read cabinet data continuous!!!");
    LOG_INFO(MAIN_TAG, "/*******************************************/");

    while (getc(stdin) != '\n');

    int c = getc(stdin);

    switch (c) {
        case '0':
            LOG_INFO(MAIN_TAG, "Serial Number: %s", bssGetSerialNumber());
            break;
        case '1':
            BssEMeterData eMeterData;
            bssGetEMeterData(eMeterData);
            LOG_INFO(MAIN_TAG, "Emeter Data: %s, Vol: %f, Cur: %f, Cos: %f, Freq: %f, Energy: %f",
                     eMeterData.m_isActive ? "ACTIVE" : "INACTIVE",
                     eMeterData.m_vol,
                     eMeterData.m_cur,
                     eMeterData.m_cos,
                     eMeterData.m_freq,
                     eMeterData.m_kwh);
            break;
        case '2':
            LOG_INFO(MAIN_TAG, "Number of cabinet %d", bssGetDeviceConfiguration().m_cabNumber);
            break;

        case '3':
            LOG_INFO(MAIN_TAG, "Number of cabinet %d", bssGetDeviceConfiguration().m_cabNumber);
            while (1){
                BssCabData cabData;
//                for(int index = 0; index < bssGetDeviceConfiguration().m_cabNumber; index++){
                    cabGetData(0, cabData);
                    LOG_ERROR(MAIN_TAG, "Cabinet Data: Door status: %d, Bp serial: %s, Bp SOC: %d",cabData.m_doorStt, cabData.m_bpSn.c_str(), cabData.m_bpSoc);
//                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            break;
        default:
            LOG_WARN(MAIN_TAG, "Please choose again");
            break;
    }

}

void test_cabinet(){
    LOG_INFO(MAIN_TAG, "/******************* TEST Cabinet *********************/");
    LOG_INFO(MAIN_TAG, "Please choose the character for Cabinet testing !!!!");
    LOG_INFO(MAIN_TAG, "Choose '0' for get all data cabinet!!!");
    LOG_INFO(MAIN_TAG, "Choose '1' for get cabinet list that EMPTY !!!");
    LOG_INFO(MAIN_TAG, "Choose '2' for get cabinet list that have BP !!!");
    LOG_INFO(MAIN_TAG, "Choose '3' for control FAN and get FAN status !!!");
    LOG_INFO(MAIN_TAG, "Choose '4' for control CHARGER SWITCH and get CHARGER SWITCH status !!!");
    LOG_INFO(MAIN_TAG, "Choose '5' for open DOOR and get DOOR status !!!");
    LOG_INFO(MAIN_TAG, "Choose '6' for get temperatures !!!");
    LOG_INFO(MAIN_TAG, "Choose '7' for assign device to BP !!!");
    LOG_INFO(MAIN_TAG, "Choose '8' for tried to assign BP to System !!!");
    LOG_INFO(MAIN_TAG, "Choose '9' for Upgrading firmware (OTA) for Slave !!!");
    LOG_INFO(MAIN_TAG, "/*******************************************/");

    LOG_INFO(MAIN_TAG, "Please Enter ID first !!!");
    while (getc(stdin) != '\n');

    int id;
    scanf("%d", &id);

    LOG_INFO(MAIN_TAG, "Cabinet %d is choose with cmd", id);

    if(id > 19 || id < 0){
        LOG_ERROR(MAIN_TAG, "Cabinet ID INVALID");
        return;
    }

    BssCabData cabData;
    uint32_t cabList[20];
    int size  = 0;
    static int value = 0;
    std::string device = "CM0001_";

    while (getc(stdin) != '\n');
    int c = getchar();

    switch (c) {
        case '0':
            cabGetData(id, cabData);
            LOG_INFO(MAIN_TAG, "Cabinet %d info: Door STT: %d, FanSTT: %d, Charger STT: %d, Temp: %d, Pogo Temps: %d,%d,%d,%d", id,
                     cabData.m_doorStt,
                     cabData.m_fanStt,
                     cabData.m_chargerSwitchStt,
                     cabData.m_temp,
                     cabData.m_pogoPinTemp[0],
                     cabData.m_pogoPinTemp[1],
                     cabData.m_pogoPinTemp[2],
                     cabData.m_pogoPinTemp[3]);
            if(!cabData.m_bpSn.empty()){
                LOG_INFO(MAIN_TAG, "Cabinet %d have BP with SN: %s, SOC: %d", id, cabData.m_bpSn.c_str(), cabData.m_bpSoc);;
            }
            break;
        case '1':
            size = cabGetEmpty(cabList);
            LOG_INFO(MAIN_TAG, "List cabinet List that Empty: %d", size);
            break;
        case '2':
            size = cabsWithBp(cabList);
            LOG_INFO(MAIN_TAG, "List cabinet List that have BP: %d", size);
            break;
        case '3':
            cabCtlFan(id, value);
            LOG_INFO(MAIN_TAG, "Fan Status: %d", cabGetFanStatus(id));
            value = !value;
            break;
        case '4':
            cabCtlChargerSwitch(id, value);
            LOG_INFO(MAIN_TAG, "Charger Switch Status: %d", cabGetChargerSwitchStatus(id));
            value = !value;
            break;
        case '5':
            cabOpenDoor(id);
            LOG_INFO(MAIN_TAG, "Door Status: %d", cabGetDoorStatus(id));
            break;
        case '6':
            cabGetTemps(id, (int32_t*)cabList);
            LOG_INFO(MAIN_TAG, "Temperatures value: %d, %d, %d, %d, %d", cabList[0],
                     cabList[1],
                     cabList[2],
                     cabList[3],
                     cabList[4]);
            break;
        case '7':
            device.append(std::to_string(id));
            cabAssignedDeviceToBp(id, device);
            LOG_INFO(MAIN_TAG, "Assigning Device to BP: %s", device.c_str());
            break;
        case '8':
            cabTryReadBp(id, 2);
            LOG_INFO(MAIN_TAG, "Tried to assigned BP: %d two time", id);
            break;
        case '9':
            cabUpgradeFw(id, "v2.0.0", "/home/pi/Documents/slave.hex");
            LOG_INFO(MAIN_TAG, "Upgrade new firmware to Slave of Cabinet %d", id);
            break;
        default:
            LOG_WARN(MAIN_TAG, "Please choose again");
            break;
    }
}

void test_bp(){
    LOG_INFO(MAIN_TAG, "/***************** Test BP ******************/");
    LOG_INFO(MAIN_TAG, "Please choose the character for Battery Pack testing !!!");
    LOG_INFO(MAIN_TAG, "Choose '0' for get all data BP !!!");
    LOG_INFO(MAIN_TAG, "Choose '1' for upgrade new firmware to BP !!!");
    LOG_INFO(MAIN_TAG, "Choose '2' for reboot BP !!!");
    LOG_INFO(MAIN_TAG, "Choose '3' for upgrade new firmware to BP continuously");
    LOG_INFO(MAIN_TAG, "/*******************************************/");

    LOG_INFO(MAIN_TAG, "Please Enter ID first !!!");
    while (getc(stdin) != '\n');

    int id;
    scanf("%d", &id);
    LOG_INFO(MAIN_TAG, "Cabinet %d is choose with cmd", id);

    if(id > 19 || id < 0){
        LOG_ERROR(MAIN_TAG, "Cabinet ID INVALID");
        return;
    }

    BpSyncData _data;
    bpSyncData(_data);

    while (getc(stdin) != '\n');

    int c = getchar();
    switch (c) {
        case '0':
            LOG_INFO(MAIN_TAG, "BP %d serial: %s, SOC:%d, SOH:%d, Cur:%d, Vol:%d, Cell:[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d], "
                               "Temp:[%d,%d,%d,%d,%d,%d,%d], Status:%d, State:%d", id,
                     _data.m_bp[id].m_sn.c_str(),
                     _data.m_bp[id].m_soc,
                     _data.m_bp[id].m_soh,
                     _data.m_bp[id].m_cur,
                     _data.m_bp[id].m_vol,
                     _data.m_bp[id].m_cellVols[0],
                     _data.m_bp[id].m_cellVols[1],
                     _data.m_bp[id].m_cellVols[2],
                     _data.m_bp[id].m_cellVols[3],
                     _data.m_bp[id].m_cellVols[4],
                     _data.m_bp[id].m_cellVols[5],
                     _data.m_bp[id].m_cellVols[6],
                     _data.m_bp[id].m_cellVols[7],
                     _data.m_bp[id].m_cellVols[8],
                     _data.m_bp[id].m_cellVols[9],
                     _data.m_bp[id].m_cellVols[10],
                     _data.m_bp[id].m_cellVols[11],
                     _data.m_bp[id].m_cellVols[12],
                     _data.m_bp[id].m_cellVols[13],
                     _data.m_bp[id].m_cellVols[14],
                     _data.m_bp[id].m_cellVols[15],
                     _data.m_bp[id].m_temps[0],
                     _data.m_bp[id].m_temps[1],
                     _data.m_bp[id].m_temps[2],
                     _data.m_bp[id].m_temps[3],
                     _data.m_bp[id].m_temps[4],
                     _data.m_bp[id].m_temps[5],
                     _data.m_bp[id].m_temps[6],

                     _data.m_bp[id].m_status,
                     _data.m_bp[id].m_state);
            break;
        case '1':
            if(_data.m_bp[id].m_sn.empty()){
                LOG_WARN(MAIN_TAG, "Cabinet %d do NOT have BP", id);
                return;
            }
            bpUpgradeFw(id, _data.m_bp[id].m_sn, "v1.0.15", "/home/pi/Documents/bp.hex");
            LOG_INFO(MAIN_TAG, "Upgrade new firmware to BP %d with sn: %s", id, _data.m_bp[id].m_sn.c_str());
            break;

        case '3':
            LOG_INFO(MAIN_TAG, "Upgrade new firmware to BP continuously");
            bpUpgradeFw(0, _data.m_bp[id].m_sn, "v1.0.15", "/home/pi/Documents/bp.hex");
            bpUpgradeFw(1, _data.m_bp[id].m_sn, "v1.0.15", "/home/pi/Documents/bp.hex");
            bpUpgradeFw(2, _data.m_bp[id].m_sn, "v1.0.15", "/home/pi/Documents/bp.hex");
            bpUpgradeFw(3, _data.m_bp[id].m_sn, "v1.0.15", "/home/pi/Documents/bp.hex");
            break;
        default:
            LOG_WARN(MAIN_TAG, "Please choose again");
            break;
    }
}

void test_charger(){
    int ret = -1;

    LOG_INFO(MAIN_TAG, "/******************TEST Charger *******************/");
    LOG_INFO(MAIN_TAG, "Please choose the character for Charger testing!!!!");
    LOG_INFO(MAIN_TAG, "Choose '0' for FORCE charge!!!");
    LOG_INFO(MAIN_TAG, "Choose '1' for FORCE discharge!!!");
    LOG_INFO(MAIN_TAG, "Choose '2' for ENABLE charger!!!");
    LOG_INFO(MAIN_TAG, "Choose '3' for DISABLE charger!!!");
    LOG_INFO(MAIN_TAG, "Choose '4' for Get charge INFO!!!");
    LOG_INFO(MAIN_TAG, "Choose '5' for Get cabinet that is charged!!!");
    LOG_INFO(MAIN_TAG, "/*******************************************/");

    while (getc(stdin) != '\n');
    LOG_INFO(MAIN_TAG, "Choose the character for charger testing");
    int c = getchar();

    LOG_INFO(MAIN_TAG, "Please choose Cabinet to activity");
    while (getc(stdin) != '\n');
    int cab;
    scanf("%d", &cab);

    if(cab > 19 || cab < 0){
        LOG_ERROR(MAIN_TAG, "Cabinet ID INVALID");
        return;
    }
    LOG_INFO(MAIN_TAG, "Cabinet %d is chose to activity", cab);

    switch (c) {
        case '0':
            ret = chargerCharge(cab);
            LOG_INFO(MAIN_TAG, "Charger is charged %s", ret < 0 ? "FAILURE" : "SUCCESS");
            break;
        case '1':
            ret = chargerDischarge(cab);
            LOG_INFO(MAIN_TAG, "Charger is discharged %s", ret < 0 ? "FAILURE" : "SUCCESS");
            break;
        case '2':
            ret = chargerEnableCab(cab, true);
            LOG_INFO(MAIN_TAG, "Charger is enable at the cabinet %d", cab);
            break;
        case '3':
            ret = chargerEnableCab(cab, false);
            LOG_INFO(MAIN_TAG, "Charger is disable at the cabinet %d", cab);
            break;
        case '4':
            LOG_INFO(MAIN_TAG, "Charger 0 info: Vol: %d, Cur: %d, State: %d",
                     chargerGetVol(0),
                     chargerGetCur(0),
                     chargerGetState(0));
            LOG_INFO(MAIN_TAG, "Charger 1 info: Vol: %d, Cur: %d, State: %d",
                     chargerGetVol(1),
                     chargerGetCur(1),
                     chargerGetState(1));
            break;
        case '5':
            LOG_INFO(MAIN_TAG, "Cabinet %d is charged at charger 0", chargerGetChargingCab(0));
            LOG_INFO(MAIN_TAG, "Cabinet %d is charged at charger 1", chargerGetChargingCab(1));
            break;
        default:
            LOG_WARN(MAIN_TAG, "Please choose again");
            break;
    }
}

void print_help(const char _c){
    switch (_c) {
        case 'b':
            test_bss();
            break;
        case 'c':
            test_cabinet();
            break;
        case 'p':
            test_bp();
            break;
        case 'z':
            test_charger();
            break;
        default:
            LOG_WARN(MAIN_TAG, "Please choose right character !!!");
            break;
    }
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

    bssLoggerInit(BSS_LOG_LEVEL::BSS_LOG_LEVEL_DEBUG, nullptr);
    bssInit(g_bssConfigDefault, &g_eventCallback, nullptr);

    int32_t cabNumber = bssGetDeviceConfiguration().m_cabNumber;
    LOG_DEBUG(MAIN_TAG, "Cabinet Number of BSS: %d", cabNumber);

    std::thread bss = std::thread(&bss_process);

    while (true){
        LOG_INFO(MAIN_TAG, "/******************** STARTING *******************/");
        LOG_INFO(MAIN_TAG, "Please choose the character for testing !!!!");
        LOG_INFO(MAIN_TAG, "Choose 'b' for BSS testing !!!");
        LOG_INFO(MAIN_TAG, "Choose 'c' for Cabinet testing !!!");
        LOG_INFO(MAIN_TAG, "Choose 'p' for BP testing !!!");
        LOG_INFO(MAIN_TAG, "Choose 'z' for Charger testing !!!");

        LOG_INFO(MAIN_TAG, "Please Enter character for testing !!!");
        int c = getchar();

        print_help(c);

        while (getc(stdin) != '\n');

        LOG_INFO(MAIN_TAG, "/******************* END *******************/");
        LOG_INFO(MAIN_TAG, "\n\n");
    }
}
