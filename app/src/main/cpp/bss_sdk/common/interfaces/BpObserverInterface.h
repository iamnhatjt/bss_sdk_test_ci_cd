//
// Created by vnbk on 06/07/2023.
//

#ifndef BSS_SDK_BPOBSERVERINTERFACE_H
#define BSS_SDK_BPOBSERVERINTERFACE_H

#include <memory>

class BpObserverInterface{
public:
    virtual void onBpConnected(const uint32_t& _id, const std::string& _sn, const std::string& _version) = 0;

    virtual void onBpDisconnected(const uint32_t& _id, const std::string& _sn) = 0;

    virtual void onBpAssignStatus(const uint32_t& _id, int32_t _status) = 0;

    virtual void onDeviceIsAssigned(const uint32_t& _id,
                                    int32_t _status,
                                    const std::string& _bp_sn,
                                    const std::string& _dev_type,
                                    const std::string& _dev_sn) = 0;
};

#endif //BSS_SDK_BPOBSERVERINTERFACE_H
