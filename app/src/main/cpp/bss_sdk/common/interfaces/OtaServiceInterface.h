//
// Created by vnbk on 12/09/2023.
//

#ifndef BSS_SDK_OTASERVICEINTERFACE_H
#define BSS_SDK_OTASERVICEINTERFACE_H

#include "OtaServiceObserverInterface.h"
#include "OtaFwManagerInterface.h"

class OtaServiceInterface{
public:
    explicit OtaServiceInterface(std::shared_ptr<OtaFwManagerInterface>& _fwManager) : m_fwManager(_fwManager){

    }

    virtual ~OtaServiceInterface() = default;

    void addObserver(const std::shared_ptr<OtaServiceObserverInterface>& _observer){
        m_observer = _observer;
    }

    virtual int32_t requestUpgrade(const uint8_t& _devType,
                                   const uint32_t& _devId,
                                   const std::string& _sn,
                                   const std::string& _newVersion,
                                   const std::string& _path) = 0;

    virtual int32_t reset() = 0;

    virtual void process() = 0;

protected:
    std::shared_ptr<OtaServiceObserverInterface> m_observer;

    std::shared_ptr<OtaFwManagerInterface> m_fwManager;
};

#endif //BSS_SDK_OTASERVICEINTERFACE_H
