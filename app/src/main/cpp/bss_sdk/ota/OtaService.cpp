//
// Created by vnbk on 08/09/2023.
//

#include "OtaService.h"
#include "Logger.h"
#include "BpComInterface.h"

#include "sm_boot_master.h"
#include "sm_ota_boot_impl.h"

#define TAG "OtaService"

void OtaService::bootMasterEventCallback(int32_t _error, int32_t _id, void* _arg){
    auto otaService = (OtaService*)_arg;
    if(!otaService || !otaService->m_currentUpgradingDev){
        LOG_ERROR(TAG, "Parameters is INVALID");
        otaService->resetUpgrading(OTA_UPGRADING_ERROR::OTA_ERR_INTERNAL);
        return;
    }
 /*   if(_id != otaService->m_currentUpgradingDev->m_devId){
        LOG_ERROR(TAG, "This device %d is NOT match with the processing device", otaService->m_currentUpgradingDev->m_devId);
        otaService->resetUpgrading(OTA_UPGRADING_ERROR::OTA_ERR_INTERNAL);
        return;
    }*/

    if(!_error){
        LOG_INFO(TAG, "Upgrading device %d is SUCCESS");
        otaService->resetUpgrading(OTA_UPGRADING_ERROR::OTA_ERR_NONE);
    }else{
        LOG_ERROR(TAG, "Upgrading device %d is FAILURE, Please check the progress upgrading on device");
        otaService->resetUpgrading(OTA_UPGRADING_ERROR::OTA_ERR_DOWNLOAD);
    }
}

void OtaService::bpCmdHandleCallback(BP_COMMAND _cmd, int32_t _isSuccess, const uint32_t& _id, void* _data, void* _arg){
    auto otaService = (OtaService*)_arg;
    if(!otaService){
        LOG_ERROR(TAG, "Parameters is INVALID");
        return;
    }
    if(_cmd == BP_COMMAND::BP_CMD_REBOOT){
        if(_isSuccess == BP_CMD_SUCCESS) {
            if (_id == otaService->m_currentUpgradingDev->m_devId &&
                otaService->m_currentUpgradingDev->m_devType == BSS_DEVICE_TYPE::BSS_DEV_TYPE_BP) {
                otaService->m_currentUpgradingDev->m_state = UPGRADING_DEV_UPGRADING;

                LOG_WARN(TAG, "Bp is reboot, Start upgrading firmware for BP %d and Turn off SYNC BP Communication Interface", _id);
                otaService->m_bpManager->getInterface()->sync(false);

                otaService->startUpgradingDev(otaService->m_currentUpgradingDev);
                return;
            }
        }else{
            otaService->resetUpgrading(OTA_UPGRADING_ERROR::OTA_ERR_REBOOT);
        }
    }
}

std::shared_ptr<OtaService> OtaService::create(const std::shared_ptr<BpManagerInterface>& _bpManager,
                                               const std::shared_ptr<CabManagerInterface>& _cabManager,
                                               std::shared_ptr<OtaFwManagerInterface> _fwManager
#ifdef BSS_VERSION_3_1_0
                                                , std::shared_ptr<ModbusMasterInterface>& _modbusMaster
#endif
                                               ) {
    auto otaService = std::shared_ptr<OtaService>(new OtaService(_bpManager,
                                                                 _cabManager,
                                                                 _fwManager
#ifdef BSS_VERSION_3_1_0
                                                                 , _modbusMaster
#endif
                                                                 ));
    if(otaService->initialized()){
        LOG_INFO(TAG, "Initialized OtaService SUCCESS");
        return otaService;
    }
    LOG_ERROR(TAG, "Initialized OtaService FAILURE");
    return nullptr;
}

OtaService::OtaService(const std::shared_ptr<BpManagerInterface> &_bpManager,
                       const std::shared_ptr<CabManagerInterface> &_cabManager,
                       std::shared_ptr<OtaFwManagerInterface> &_fwManager
#ifdef BSS_VERSION_3_1_0
        , std::shared_ptr<ModbusMasterInterface> &_modbusMaster
#endif
) : OtaServiceInterface(_fwManager),
    m_bpManager(_bpManager),
    m_cabManager(_cabManager),
    m_bootMaster(nullptr) {

    m_currentUpgradingDev = nullptr;

#ifdef BSS_VERSION_3_1_0
    m_mbMaster = _modbusMaster;
#endif
}

bool OtaService::initialized() {
    if(!m_bpManager || !m_cabManager){
        LOG_ERROR(TAG, "Missing parameters, please check again");
        return false;
    }

    if(m_fwManager) {
        m_fwManager->loadFw(BSS_DEVICE_TYPE::BSS_DEV_TYPE_BP);
        m_fwManager->loadFw(BSS_DEVICE_TYPE::BSS_DEV_TYPE_SLAVE);
    }

    m_bootMaster = sm_boot_master_create(bootMasterEventCallback, this);

    if(!m_bootMaster){
        LOG_ERROR(TAG, "Create Boot Master FAILURE");
        return false;
    }

    return true;
}

int32_t OtaService::requestUpgrade(const uint8_t &_devType,
                                   const uint32_t& _devId,
                                   const std::string& _sn,
                                   const std::string &_newVersion,
                                   const std::string &_path) {
    if(_devType >= BSS_DEVICE_TYPE::BSS_DEV_TYPE_NUMBER || _newVersion.empty() || _path.empty()){
        LOG_ERROR(TAG, "Request upgrade FAILURE, Reason: Params INVALID");
        return -1;
    }

    auto item = std::make_shared<UpgradingDevInfo>(_devType, _devId, _sn, _newVersion, _path);

    std::lock_guard<std::mutex> lockGuard(m_lock);
    m_upgradingDevs.push(item);
    LOG_INFO(TAG, "New device is push to queue and wait to upgrade. Device ID: %d, new version: %s, SN: %s, Path: %s", _devId,
             _newVersion.c_str(),
             _sn.c_str(),
             _path.c_str());
    return 0;
}

int32_t OtaService::reset() {


    if(m_observer){
        LOG_INFO(TAG, "Finished upgrading firmware, Notify to observer");
        m_observer->onUpgradingFinished();
    }
    return 0;
}

void OtaService::resetUpgrading(OTA_UPGRADING_ERROR _error) {
    if(m_observer){
        LOG_INFO(TAG, "Notify upgrading error %s to Observer", otaUpgradingErrorToString(_error).c_str());
        m_observer->onUpgradingStatus(m_currentUpgradingDev->m_devType,
                                      m_currentUpgradingDev->m_devId,
                                      m_currentUpgradingDev->m_sn,
                                      m_currentUpgradingDev->m_newVersion,
                                     _error,
                                     otaUpgradingErrorToString(_error));
    }

    std::lock_guard<std::mutex> lockGuard(m_lock);
    m_cabManager->setUpgradingState(m_currentUpgradingDev->m_devId, false);
    m_currentUpgradingDev.reset();

    m_bpManager->getInterface()->sync(true);

    if(!m_upgradingDevs.empty()){
        m_upgradingDevs.front()->m_state = UPGRADING_DEV_WAITING_ONLINE;
        m_upgradingDevs.front()->m_timeout.reset(UPGRADING_DEVICE_WAITING_ONLINE_AGAIN_TIMEOUT);
    }else{
        if(m_observer){
            LOG_INFO(TAG, "Finished upgrading firmware, Notify to observer");
            m_observer->onUpgradingFinished();
        }
    }
}

int32_t OtaService::prepareUpgradingDev(const std::shared_ptr<UpgradingDevInfo>& _upgradingDev) {
    if(_upgradingDev->m_devType == BSS_DEVICE_TYPE::BSS_DEV_TYPE_BP){
        auto bpData = m_bpManager->getData(_upgradingDev->m_devId);
        if(bpData.m_sn == _upgradingDev->m_sn && bpData.m_version == _upgradingDev->m_newVersion){
            LOG_INFO(TAG, "Bp %s is using this firmware %s", bpData.m_sn.c_str(), bpData.m_version.c_str());
//            resetUpgrading(OTA_UPGRADING_ERROR::OTA_ERR_NONE);
        }

        if(m_bpManager->setCmd(_upgradingDev->m_devId, BP_COMMAND::BP_CMD_REBOOT, nullptr, bpCmdHandleCallback, this, true) < 0){
            LOG_ERROR(TAG, "Reboot device %d FAILURE", _upgradingDev->m_devId);
            resetUpgrading(OTA_UPGRADING_ERROR::OTA_ERR_REBOOT);
            return -1;
        }

        m_cabManager->setUpgradingState(_upgradingDev->m_devId, true);

        _upgradingDev->m_state = UPGRADING_DEV_WAITING_REBOOT;
        _upgradingDev->m_timeout.reset(UPGRADING_DEVICE_REBOOT_TIMEOUT);

        LOG_INFO(TAG, "Start upgrading firmware on BP %d: %s, Waiting BP reboot", _upgradingDev->m_devId, _upgradingDev->m_sn.c_str());

        return 0;

    }else if(_upgradingDev->m_devType == BSS_DEVICE_TYPE::BSS_DEV_TYPE_SLAVE){
        /// TODO: Add with Slave 2.0.0
        LOG_WARN(TAG, "BSS version 3.0.0 is NOT support upgrade SLAVE module");
        return -1;
    }else if(_upgradingDev->m_devType == BSS_DEVICE_TYPE::BSS_DEV_TYPE_MASTER){
        LOG_WARN(TAG, "BSS is NOT support upgrade MASTER module");
        return -1;
    }else if(_upgradingDev->m_devType == BSS_DEVICE_TYPE::BSS_DEV_TYPE_PMM){
        LOG_WARN(TAG, "BSS is NOT support upgrade PMM module");
        return -1;
    }else{
        LOG_ERROR(TAG, "Device Type is NOT support OTA, Please check again");
        return -1;
    }
}

int32_t OtaService::startUpgradingDev(const std::shared_ptr<UpgradingDevInfo> &_upgradingDev) {
    auto bootInput= sm_get_file_boot_input(_upgradingDev->m_path.c_str());
    auto bootOutput= sm_get_co_boot_output();

    if(!bootInput || !bootOutput){
        LOG_ERROR(TAG, "Create Boot interface FAILURE");
        resetUpgrading(OTA_UPGRADING_ERROR::OTA_ERR_INTERNAL);
        return -1;
    }

    if(sm_boot_master_add_slave(m_bootMaster, /*(int32_t)_upgradingDev->m_devId +*/ 4, bootInput, bootOutput) < 0){
        LOG_ERROR(TAG, "Add Boot slave FAILURE");
        bootOutput->m_proc->free(bootOutput);
        bootInput->free();

        resetUpgrading(OTA_UPGRADING_ERROR::OTA_ERR_INTERNAL);
        return -1;
    }

    return 0;
}

void OtaService::process() {
    if(!m_upgradingDevs.empty() && !m_currentUpgradingDev){
        m_lock.lock();

        auto upgradingDev = m_upgradingDevs.front();

        if(upgradingDev->m_state == UPGRADING_DEVICE_STATE::UPGRADING_DEV_WAITING_ONLINE && !upgradingDev->m_timeout.getRemainTime()){
            LOG_ERROR(TAG, "The device %d: %s is TIMEOUT while waiting online", upgradingDev->m_devId, upgradingDev->m_sn.c_str());
            if(m_observer){
                LOG_ERROR(TAG, "Notify upgrading error %s to Observer", otaUpgradingErrorToString(OTA_ERR_TIMEOUT).c_str());
                m_observer->onUpgradingStatus(upgradingDev->m_devType,
                                              upgradingDev->m_devId,
                                              upgradingDev->m_sn,
                                              upgradingDev->m_newVersion,
                                              OTA_ERR_TIMEOUT,
                                              otaUpgradingErrorToString(OTA_ERR_TIMEOUT));
            }

            m_upgradingDevs.pop();

            if(m_observer && m_upgradingDevs.empty()){
                LOG_INFO(TAG, "Finished upgrading firmware, Notify to observer");
                m_observer->onUpgradingFinished();
            }

            m_lock.unlock();
            return;
        }

        if(upgradingDev->m_devType == BSS_DEVICE_TYPE::BSS_DEV_TYPE_BP) {
            if (m_cabManager->cabHaveBp(upgradingDev->m_devId) &&
                m_cabManager->getCab(upgradingDev->m_devId)->m_data.m_bpSn == upgradingDev->m_sn) {

                m_currentUpgradingDev = upgradingDev;
                m_upgradingDevs.pop();

                if (prepareUpgradingDev(m_currentUpgradingDev) < 0) {
                    m_currentUpgradingDev.reset();
                }
            }
        }

        m_lock.unlock();
    }

    if(m_currentUpgradingDev){
        if(m_currentUpgradingDev->m_state == UPGRADING_DEVICE_STATE::UPGRADING_DEV_WAITING_REBOOT && !m_currentUpgradingDev->m_timeout.getRemainTime()){
            LOG_ERROR(TAG, "The device is NOT reboot, Timeout");
            resetUpgrading(OTA_UPGRADING_ERROR::OTA_ERR_TIMEOUT);
            return;
        }

#ifdef BSS_VERSION_3_1_0
        m_coModbusIf->process();
#endif

        if(m_currentUpgradingDev->m_state == UPGRADING_DEVICE_STATE::UPGRADING_DEV_UPGRADING){
            sm_boot_master_process(m_bootMaster);
        }
    }
}