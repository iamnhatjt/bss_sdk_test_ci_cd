//
// Created by vnbk on 08/09/2023.
//

#ifndef BSS_SDK_OTASERVICE_H
#define BSS_SDK_OTASERVICE_H

#include <queue>
#include <mutex>

#include "OtaServiceInterface.h"
#include "BpManagerInterface.h"
#include "CabManagerInterface.h"

#include "sm_boot_master.h"
#include "bss_data_def.h"

#ifdef BSS_VERSION_3_1_0
#include "OtaCoModbusIfConfig.h"
#endif


#define UPGRADING_DEVICE_REBOOT_TIMEOUT                     5000
#define UPGRADING_DEVICE_WAITING_ONLINE_AGAIN_TIMEOUT       30000

class OtaService : public OtaServiceInterface{
public:
    enum UPGRADING_DEVICE_STATE{
        UPGRADING_DEV_IDLE = 0,
        UPGRADING_DEV_WAITING_ONLINE,
        UPGRADING_DEV_WAITING_REBOOT,
        UPGRADING_DEV_UPGRADING,
    };

    static std::shared_ptr<OtaService> create(const std::shared_ptr<BpManagerInterface>& _bpManager,
                                              const std::shared_ptr<CabManagerInterface>& _cabManager,
                                              std::shared_ptr<OtaFwManagerInterface> _fwManager
#ifdef BSS_VERSION_3_1_0
                                                , std::shared_ptr<ModbusMasterInterface>& _modbusMaster
#endif
                                              );

    static void bpCmdHandleCallback(BP_COMMAND _cmd, int32_t _isSuccess, const uint32_t& _id, void* _data, void* _arg);
    static void bootMasterEventCallback(int32_t _error, int32_t _id, void* _arg);

    int32_t requestUpgrade(const uint8_t& _devType,
                           const uint32_t& _devId,
                           const std::string& _sn,
                           const std::string& _newVersion,
                           const std::string& _path) override;

    int32_t reset() override;

    void process() override;
private:
    struct UpgradingDevInfo{
        uint8_t m_devType;
        int32_t m_devId;
        std::string m_sn;
        std::string m_newVersion;
        uint8_t m_state;
        WaitTimer m_timeout;
        std::string m_path;

        UpgradingDevInfo(uint8_t _devType,
                         int32_t _devId,
                         const std::string& _sn,
                         const std::string& _newVersion,
                         const std::string& _path,
                         uint8_t _state = UPGRADING_DEVICE_STATE::UPGRADING_DEV_IDLE,
                         int32_t _timeout = UPGRADING_DEVICE_REBOOT_TIMEOUT) : m_timeout(_timeout),
                                                                     m_devType(_devType),
                                                                     m_devId(_devId),
                                                                     m_sn(_sn),
                                                                     m_path(_path),
                                                                     m_newVersion(_newVersion),
                                                                     m_state(_state){
        }
    };

    OtaService(const std::shared_ptr<BpManagerInterface>& _bpManager,
               const std::shared_ptr<CabManagerInterface>& _cabManager,
               std::shared_ptr<OtaFwManagerInterface>& _fwManager
#ifdef BSS_VERSION_3_1_0
            , std::shared_ptr<ModbusMasterInterface>& _modbusMaster
#endif
               );

    bool initialized();

    void resetUpgrading(OTA_UPGRADING_ERROR _error = OTA_ERR_NONE);

    int32_t prepareUpgradingDev(const std::shared_ptr<UpgradingDevInfo>& _upgradingDev);

    int32_t startUpgradingDev(const std::shared_ptr<UpgradingDevInfo>& _upgradingDev);

    std::shared_ptr<BpManagerInterface> m_bpManager;
    std::shared_ptr<CabManagerInterface> m_cabManager;

#ifdef BSS_VERSION_3_1_0
    std::shared_ptr<ModbusMasterInterface> m_mbMaster;
    std::shared_ptr<OtaCoModbusIfConfig> m_coModbusIf;
#endif

    sm_boot_master_t* m_bootMaster;

    std::queue<std::shared_ptr<UpgradingDevInfo>> m_upgradingDevs;
    std::shared_ptr<UpgradingDevInfo> m_currentUpgradingDev;

    std::mutex m_lock;
};


#endif //BSS_SDK_OTASERVICE_H
