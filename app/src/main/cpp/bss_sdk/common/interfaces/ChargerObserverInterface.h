//
// Created by vnbk on 24/07/2023.
//

#ifndef BSS_SDK_CHARGEROBSERVERINTERFACE_H
#define BSS_SDK_CHARGEROBSERVERINTERFACE_H

#include <memory>

typedef enum {
    BP_IS_FULL,
    BP_OVER_TEMP,
    BP_IS_REMOVED,
    CAB_OVER_TEMP,
    CHARGER_OVER_CUR,
    CHARGER_IS_PROTECTED,
    FORCE_DISCHARGED,
}CHARGER_DISCHARGED_REASON;

class ChargerObserverInterface{
public:
    virtual ~ChargerObserverInterface() = default;

    virtual void onCharged(const uint8_t& _id, const uint32_t& _cabId, int32_t _isSuccess) = 0;

    virtual void onDischarged(const uint8_t& _id, const uint32_t& _cabId, CHARGER_DISCHARGED_REASON _reason) = 0;
};

static inline std::string dischargedReasonToString(CHARGER_DISCHARGED_REASON _reason){
    switch (_reason) {
        case BP_IS_FULL:
            return "BP_IS_FULL";
        case BP_OVER_TEMP:
            return "BP_OVER_TEMPERATURE";
        case BP_IS_REMOVED:
            return "BP_IS_REMOVED";
        case CAB_OVER_TEMP:
            return "CAB_OVER_TEMPERATURE";
        case CHARGER_OVER_CUR:
            return "CHARGER_OVER_CURRENT";
        case CHARGER_IS_PROTECTED:
            return "CHARGER_IS_PROTECTED";
        case FORCE_DISCHARGED:
            return "FORCE_DISCHARGED";
        default:
            return "UNKNOWN";
    }
}

#endif //BSS_SDK_CHARGEROBSERVERINTERFACE_H
