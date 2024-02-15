#include <jni.h>
#include <string>
#include "bss_api.h"
#include <thread>
#include <queue>
#include <mutex>
#include <memory>
#include <android/log.h>

#include <iostream>
#include <sstream>

static JavaVM *g_jvm = nullptr;
static JNIEnv* g_env = nullptr;
static jobject g_jnvBackgroudService = nullptr;

typedef enum{
    BSS_CAB_CHANGED_DOOR_STT = 0,
    BSS_CAB_CHANGED_FAN_STT = 1,
    BSS_CAB_CHANGED_CHARGER_STT = 2,
    BSS_CAB_TRY_READ_BP_INFO = 3,
    BSS_BP_CONNECTED = 4,
    BSS_BP_DISCONNECTED = 5,
    BSS_EVENT_NUMBER = 6,
    BSS_EVENT_ASSIGN_BATTERY = 7
}BSS_EVENT;

typedef struct {
    int32_t m_id;
    BSS_EVENT m_type;
    std::string m_value;
}BssEvent;

static std::queue<std::shared_ptr<BssEvent>> g_bssEvents;
static std::mutex g_lock;
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-lib", __VA_ARGS__))

void bss_log_puts(const char* _log){
    __android_log_print(ANDROID_LOG_DEBUG, "BSS_DEVICE", "%s", _log);
}

/// BSS-API Callback Function

/***
 * 
 * @param _id 
 * @param _isSuccess 
 * @param _arg 
 */

void bss_onTryReadBp(uint32_t _id, uint8_t _isSuccess, void* _arg){
    printf("Cabinet %d tried read BP %s\n", _id, _isSuccess ? "SUCCESS" : "FAILURE");

    std::string valueString = _isSuccess ? "SUCCESS" : "FAILURE";

    g_lock.lock();
    auto event = std::make_shared<BssEvent>();
    event->m_type = BSS_CAB_TRY_READ_BP_INFO;

    event->m_value = "{\"isSuccess\":";
    event->m_value.append(std::to_string(_isSuccess));
    event->m_value.append("}");

    g_bssEvents.push(event);

    g_lock.unlock();

   /* JNIEnv* env;
    g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);

    if(!env)
        return;

    jclass objClass = env->GetObjectClass(g_jnvBackgroudService);

    jmethodID callBackJava = env->GetMethodID(objClass, "callBackOnRetryBp","(Ljava/lang/String;)V");

    env->CallVoidMethod(g_jnvBackgroudService, callBackJava, env->NewStringUTF(valueString.c_str()));*/
}

void bss_onCabDoorChangedStatus(uint32_t _id, uint8_t _isSuccess, void* _arg){
    g_lock.lock();
    auto event = std::make_shared<BssEvent>();
    event->m_type = BSS_CAB_CHANGED_DOOR_STT;

    event->m_value = "{\"status\":";
    event->m_value.append(std::to_string(_isSuccess));
    event->m_value.append(",\"id\":");
    event->m_value.append(std::to_string(_id));
    event->m_value.append("}");
    g_bssEvents.push(event);

    g_lock.unlock();
}


void bss_onCabFanChangedStatus(uint32_t _id, uint8_t _isSuccess, void* _arg){
    printf("Fan %d status is changed: %s\n", _id, _isSuccess ? "SUCCESS" : "FAILURE");
}
void bss_onCabChargerChangedStatus(uint32_t _id, uint8_t _isSuccess, void* _arg){
    printf("Charger %d status is changed: %s\n", _id, _isSuccess ? "SUCCESS" : "FAILURE");
}
void bss_onCabChangedState(uint32_t _id, uint8_t _isSuccess, void* _arg){
    printf("Cabinet %d state is changed: %s\n", _id, _isSuccess ? "SUCCESS" : "FAILURE");
}

void bss_onBpConnected(uint32_t _id, const std::string& _sn, const std::string& _version, void* _arg){
    printf("BP %s is connected at the cabinet %d\n", _sn.c_str(),  _id);

    std::string valueString = "SUCCESS";

    g_lock.lock();
    auto event = std::make_shared<BssEvent>();
    event->m_type = BSS_BP_CONNECTED;


    event->m_value = "{\"sn\":\"";
    event->m_value.append(_sn);
    event->m_value.append("\",\"id\":");
    event->m_value.append(std::to_string(_id));
    event->m_value.append("}");

    g_bssEvents.push(event);

    g_lock.unlock();
}
void bss_onBpDisconnected(uint32_t _id, const std::string& _sn, void* _arg){
    printf("BP %s is disconnected at the cabinet %d\n", _sn.c_str(),  _id);

    g_lock.lock();
    auto event = std::make_shared<BssEvent>();
    event->m_type = BSS_BP_DISCONNECTED;

    event->m_value = "{\"sn\":\"";
    event->m_value.append(_sn);
    event->m_value.append("\",\"id\":");
    event->m_value.append(std::to_string(_id));
    event->m_value.append("}");

    g_bssEvents.push(event);

    g_lock.unlock();
}
void bss_onDeviceIsAssignedToBp(uint32_t _id, uint8_t _isSuccess, const std::string& _sn, void* _arg){
    printf("com.selex.bssStation.service.Device %s is assigned in the BP %d\n", _sn.c_str(),  _id);
    //std::string valueString = _isSuccess ? "SUCCESS" : "FAILURE";

    g_lock.lock();
    auto event = std::make_shared<BssEvent>();
    event->m_type = BSS_EVENT_ASSIGN_BATTERY;

    event->m_value = "{\"sn\":\"";
    event->m_value.append(_sn);
    event->m_value.append("\",\"id\":");
    event->m_value.append(std::to_string(_id));
    event->m_value.append(",\"isSuccess\":");
    event->m_value.append(std::to_string(_isSuccess));
    event->m_value.append("}");

    g_bssEvents.push(event);

    g_lock.unlock();

}
void bss_onBssSyncData(BssSyncData& _data,  void* _arg){
    printf("onBssSyncData\n");
}

static BssEventCallback g_bssEventCallback = {
        .m_onBssSyncData = bss_onBssSyncData,
        .m_onChangedDoorStt = bss_onCabDoorChangedStatus,
        .m_onChangedFanStt = bss_onCabFanChangedStatus,
        .m_onChangedChargerStt = bss_onCabChargerChangedStatus,
        .m_onChangedConnection = bss_onCabChangedState,
        .m_onTryReadBp = bss_onTryReadBp,
        .m_onBpConnected = bss_onBpConnected,
        .m_onBpDisconnected = bss_onBpDisconnected,
        .m_onDeviceIsAssignedToBp = bss_onDeviceIsAssignedToBp
};

extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_setDoorOpenFromJNI(JNIEnv *env, jclass clazz,
                                                                     jint id) {
    return cabOpenDoor(id);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getStateBssFromJNI(JNIEnv *env, jclass clazz) {
    return 1;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getNumberCabinetFromJNI(JNIEnv *env,
                                                                          jclass clazz) {
    BssDeviceConfiguration bssConfiguration = bssGetDeviceConfiguration();
    return bssConfiguration.m_cabNumber;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getReadyPackFromJNI(JNIEnv *env, jclass clazz) {
/*  BssDeviceInfo deviceInfo;
  bssGetDeviceInfo(deviceInfo);
  return deviceInfo.m_readyPack;*/
return 2;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getTotalPackFromJNI(JNIEnv *env, jclass clazz) {
   /* BssDeviceInfo deviceInfo;
    bssGetDeviceInfo(deviceInfo);
    return deviceInfo.m_totalPack;*/
    return 2;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getDoorStateFromJNI(JNIEnv *env, jclass clazz,
                                                                      jint id) {
    return cabGetDoorStatus(id);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getFanStateFromJNI(JNIEnv *env, jclass clazz,
                                                                     jint id) {
    return cabGetFanStatus(id);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getOpStateFromJNI(JNIEnv *env, jclass clazz,
                                                                    jint id) {
    BssCabData cabData;
    cabGetData(id, cabData);
    return cabData.m_isConnected;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getStateCabinetFromJNI(JNIEnv *env, jclass clazz,
                                                                         jint id) {
    return cabIsConnected(id);
}
extern "C"
JNIEXPORT jintArray JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getTempCabinetListFromJNI(JNIEnv *env,
                                                                            jclass clazz, jint id) {
    BssCabData cabData;
    cabGetData(id, cabData);

    int arr[5]={(int) cabData.m_temp,(int) cabData.m_pogoPinTemp[0],(int) cabData.m_pogoPinTemp[1],(int) cabData.m_pogoPinTemp[2],(int) cabData.m_pogoPinTemp[3]};
    jintArray ret = env->NewIntArray(5);

    env->SetIntArrayRegion(ret,0,5,arr);

    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getIsChargedFromJNI(JNIEnv *env, jclass clazz,
                                                                      jint id) {
    BssCabData cabData;
    cabGetData(id, cabData);

    return cabData.m_isCharged;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getTempCabinetThresholdFromJNI(JNIEnv *env,
                                                                                 jclass clazz) {
    return bssGetDeviceConfiguration().m_chargerConfig.m_tempStartCharging;
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getSerialBatteryFromJNI(JNIEnv *env, jclass clazz,
                                                                          jint id) {
    BssCabData cabData;
    cabGetData(id, cabData);
    std::string sn = cabData.m_bpSn;
    return env->NewStringUTF(sn.c_str());
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getSocBatteryFromJNI(JNIEnv *env, jclass clazz,
                                                                       jint id) {
    BssCabData cabData;
    cabGetData(id, cabData);

    return cabData.m_bpSoc;

//    BpSyncData bpData;
//    bpSyncData(bpData);
//
//    return bpData.m_bp[id].m_soc;
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getFactoryDateFromJNI(JNIEnv *env, jclass clazz) {
    SelexManufactureInfo selexManufactureInfo = bssGetManufacture();

    return env->NewStringUTF(selexManufactureInfo.m_factoryDate.c_str());
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getHwVersionFromJNI(JNIEnv *env, jclass clazz) {
    SelexManufactureInfo elexManufactureInfo  = bssGetManufacture();
    return env->NewStringUTF(elexManufactureInfo.m_hwVersion.c_str());
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getLotNumberFromJNI(JNIEnv *env, jclass clazz) {
    SelexManufactureInfo elexManufactureInfo = bssGetManufacture();
    return env->NewStringUTF(elexManufactureInfo.m_lotNumber.c_str());
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getModelFromJNI(JNIEnv *env, jclass clazz) {
    SelexManufactureInfo elexManufactureInfo = bssGetManufacture();
    return env->NewStringUTF(elexManufactureInfo.m_model.c_str());
}
extern "C"
JNIEXPORT void JNICALL
Java_com_selex_bssStation_service_BackgroundService_bssInitFromJNI(JNIEnv *env, jobject thiz) {
    BssDeviceConfiguration bssConfiguration;
//
//    bssConfiguration.m_version = BSS_VERSION_3_0_0; //BSS_VERSION_3_1_0
//    bssConfiguration.m_chargerConfig.m_chargerNumber = 2;
//
////    bssConfiguration.m_emeterEnable = true;

    bssInit(bssConfiguration, &g_bssEventCallback, nullptr);
    bssLoggerInit(BSS_LOG_LEVEL::BSS_LOG_LEVEL_DEBUG, bss_log_puts);

    env->GetJavaVM(&g_jvm); //store jvm reference for later call
    g_jnvBackgroudService =  env->NewGlobalRef(thiz);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_processBssFromJNY(JNIEnv *env, jclass clazz) {
    bssProcess();

    if(!g_bssEvents.empty()){
        g_lock.lock();
        auto event = g_bssEvents.front();
        g_bssEvents.pop();
        g_lock.unlock();

        JNIEnv* env;
        g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);

        if(!env){
            return;
        }

        jclass objClass = env->GetObjectClass(g_jnvBackgroudService);

        jmethodID callBackJava = env->GetMethodID(objClass, "callBackBssFromJNI","(IILjava/lang/String;)V");

        env->CallVoidMethod(g_jnvBackgroudService, callBackJava, (int)event->m_id, (int)event->m_type, env->NewStringUTF(event->m_value.c_str()));
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_cabAssignedDevicesToBpFromJNI(JNIEnv *env,
                                                                                jclass clazz,
                                                                                jint id,
                                                                                jstring serial) {
    jboolean isCopy;
    std::string ev_sn = env->GetStringUTFChars(serial, &isCopy);
    return  cabAssignedDeviceToBp(id, ev_sn);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getInfoBp(JNIEnv *env, jclass clazz, jint id) {
    BpSyncData bpSycData;
    bpSyncData(bpSycData);

    std::ostringstream payload;


    payload << R"({"soc":)"         << bpSycData.m_bp[id].m_soc;
    payload << R"(,"soh":)"         << bpSycData.m_bp[id].m_soh;
    payload << R"(,"cycle":)"       << bpSycData.m_bp[id].m_cycle;
    payload << R"(,"vol":)"         << bpSycData.m_bp[id].m_vol;
    payload << R"(,"cur":)"         << bpSycData.m_bp[id].m_cur;
    payload << R"(,"op_state":)"    << bpSycData.m_bp[id].m_state;
    payload << R"(,"status":)"      << bpSycData.m_bp[id].m_status;
    payload << R"(,"version":")"      << bpSycData.m_bp[id].m_version;
    //payload << R"(","assignedSn":")"      << bpSycData.m_bp[id].m_assignedSn;
//    payload << R"(,"temps":[)";
//    for(int index = 0; index < BP_CELL_TEMP_SIZE; index++){
//        payload << R"()"   << bpSycData.m_bp[id].m_temps[index];
//        if(index < (BP_CELL_TEMP_SIZE - 1))
//            payload << R"(,)";
//    }
//
//    payload << R"(],"cells_vol":[)" ;
//
//    for(int index = 0; index < BP_CELL_VOL_SIZE; index++){
//        payload << R"()"   << bpSycData.m_bp[id].m_cellVols[index];
//        if(index < (BP_CELL_VOL_SIZE - 1))
//            payload << R"(,)";
//    }

    payload << R"("})";

    return env->NewStringUTF(payload.str().c_str());;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getAssignedSn(JNIEnv *env, jclass clazz,
                                                                jint id) {
    BpSyncData bpSycData;
    bpSyncData(bpSycData);

    try {
        return env->NewStringUTF( bpSycData.m_bp[id].m_assignedSn.c_str());
    }catch(int ret) {
        return env->NewStringUTF( "");
    }

//    return env->NewStringUTF( bpSycData.m_bp[id].m_assignedSn.c_str());
}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getTempBp(JNIEnv *env, jclass clazz, jint id) {
    BpSyncData bpSycData;
    bpSyncData(bpSycData);

    int arr[BP_CELL_TEMP_SIZE]={};
    jintArray ret = env->NewIntArray(BP_CELL_TEMP_SIZE);

    for (int index = 0; index < BP_CELL_TEMP_SIZE; index++) {
        arr[index] = bpSycData.m_bp[id].m_temps[index];
    }

    env->SetIntArrayRegion(ret,0,BP_CELL_TEMP_SIZE,arr);
    LOGI("%f", ret);
    return ret;
}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getCellVol(JNIEnv *env, jclass clazz, jint id) {
    BpSyncData bpSycData;
    bpSyncData(bpSycData);

    int arr[BP_CELL_VOL_SIZE]={};
    jintArray ret = env->NewIntArray(BP_CELL_VOL_SIZE);

    for (int index = 0; index < BP_CELL_VOL_SIZE; index++) {
        arr[index] = bpSycData.m_bp[id].m_cellVols[index];
    }

    env->SetIntArrayRegion(ret,0,BP_CELL_VOL_SIZE,arr);
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_chargerGetNumberBssJNI(JNIEnv *env, jclass clazz) {
    return chargerGetNumber();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_chargerGetVol(JNIEnv *env, jclass clazz,
                                                                jint id) {
    return chargerGetVol(id);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_chargerGetCur(JNIEnv *env, jclass clazz,
                                                                jint id) {
    return chargerGetCur(id);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_chargerGetChargingCab(JNIEnv *env, jclass clazz,
                                                                        jint id) {
    return chargerGetChargingCab(id);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_chargerCharge(JNIEnv *env, jclass clazz,
                                                                jint cab_id) {
    return chargerCharge(cab_id);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_chargerDischarge(JNIEnv *env, jclass clazz,
                                                                    jint cab_id) {
    return chargerDischarge(cab_id);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_chargerCabEnable(JNIEnv *env, jclass clazz,
                                                                   jint id, jint cab_id,
                                                                   jboolean boolean) {
    return chargerEnableCab(cab_id, boolean);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_chargerGetState(JNIEnv *env, jclass clazz,
                                                                  jint id) {
    return chargerGetState(id);
}
extern "C"
JNIEXPORT jfloat JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_emeterGetVol(JNIEnv *env, jclass clazz) {
    BssEMeterData eMeterData;
    bssGetEMeterData(eMeterData);

    return eMeterData.m_vol;
}
extern "C"
JNIEXPORT jfloat JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_emeterGetCur(JNIEnv *env, jclass clazz) {
    BssEMeterData eMeterData;
    bssGetEMeterData(eMeterData);

    return eMeterData.m_cur;
}
extern "C"
JNIEXPORT jfloat JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_emeterGetCos(JNIEnv *env, jclass clazz) {
    BssEMeterData eMeterData;
    bssGetEMeterData(eMeterData);

    return eMeterData.m_cos;
}
extern "C"
JNIEXPORT jfloat JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_emeterGetFreq(JNIEnv *env, jclass clazz) {
    BssEMeterData eMeterData;
    bssGetEMeterData(eMeterData);

    return eMeterData.m_freq;
}
extern "C"
JNIEXPORT jfloat JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_emeterGetEnergy(JNIEnv *env, jclass clazz) {
    BssEMeterData eMeterData;
    bssGetEMeterData(eMeterData);

    return eMeterData.m_kwh;
}



extern "C"
JNIEXPORT jint JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_tryReadBp(JNIEnv *env, jclass clazz, jint id,
                                                            jint count) {
    return cabTryReadBp(id, count);
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_selex_bssStation_service_DeviceServiceKt_getListBp(JNIEnv *env, jclass clazz) {
    return env->NewStringUTF( cabsAllBpData().c_str());
}