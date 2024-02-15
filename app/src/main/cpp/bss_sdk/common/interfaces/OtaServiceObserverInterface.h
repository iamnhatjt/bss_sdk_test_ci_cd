//
// Created by vnbk on 12/09/2023.
//

#ifndef BSS_SDK_OTASERVICEOBSERVERINTERFACE_H
#define BSS_SDK_OTASERVICEOBSERVERINTERFACE_H

#include <memory>

typedef enum {
    OTA_ERR_NONE        = 0,
    OTA_ERR_INTERNAL    = -1,
    OTA_ERR_TIMEOUT     = -2,
    OTA_ERR_REBOOT      = -3,
    OTA_ERR_DOWNLOAD    = -4,
    OTA_ERR_UNKNOWN     = -5
}OTA_UPGRADING_ERROR;

typedef enum {
    OTA_STT_PENDING,
    OTA_STT_WAITING_REBOOT,
    OTA_STT_UPGRADING,
    OTA_STT_FINISHED,
}OTA_UPGRADING_STATUS;

class OtaServiceObserverInterface {
public:
    virtual void onUpgradingReady(const uint8_t &_devType, const uint32_t &_id, bool _isReady) = 0;

    virtual void onUpgradingFinished() = 0;

    virtual void onUpgradingStatus(const uint8_t &_devType,
                                  const uint32_t &_id,
                                  const std::string &_sn,
                                  const std::string& _newVersion,
                                  const int32_t &_error,
                                  const std::string &_reason) = 0;
};

static inline std::string otaUpgradingErrorToString(OTA_UPGRADING_ERROR _error){
    switch (_error) {
        case OTA_ERR_NONE:
            return "OTA_ERR_NONE";
        case OTA_ERR_INTERNAL:
            return "OTA_ERR_INTERNAL";
        case OTA_ERR_TIMEOUT:
            return "OTA_ERR_TIMEOUT";
        case OTA_ERR_REBOOT:
            return "OTA_ERR_REBOOT";
        case OTA_ERR_DOWNLOAD:
            return "OTA_ERR_DOWNLOAD";
        case OTA_ERR_UNKNOWN:
            return "OTA_ERR_UNKNOWN";
        default:
            return "OTA_ERR_UNKNOWN";
    }
}

static inline std::string otaUpgradingStatusToString(OTA_UPGRADING_STATUS _status){
    switch (_status) {
        case OTA_STT_PENDING:
            return "OTA_STT_PENDING";
        case OTA_STT_WAITING_REBOOT:
            return "OTA_STT_WAITING_REBOOT";
        case OTA_STT_UPGRADING:
            return "OTA_STT_UPGRADING";
        case OTA_STT_FINISHED:
            return "OTA_STT_FINISHED";
        default:
            return "OTA_STT_UNKNOWN";
    }
}

#endif //BSS_SDK_OTASERVICEOBSERVERINTERFACE_H
