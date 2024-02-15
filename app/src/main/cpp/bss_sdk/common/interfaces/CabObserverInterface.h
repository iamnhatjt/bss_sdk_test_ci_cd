//
// Created by vnbk on 01/06/2023.
//

#ifndef BSS_PROCESS_CABINETOBSERVERINTERFACE_H
#define BSS_PROCESS_CABINETOBSERVERINTERFACE_H

#include <memory>

class CabObserverInterface{
public:
    virtual ~CabObserverInterface() = default;

    virtual void onReady() = 0;

    virtual void onCabConnectionChanged(uint32_t _id, uint32_t _newSate) = 0;

    virtual void onCabChangedDoorStatus(uint32_t _id, uint32_t _newStt) = 0;
    
    virtual void onCabChangedFanStatus(uint32_t _id, uint32_t _newStt) = 0;
    
    virtual void onCabChangedChargerStatus(uint32_t _id, uint32_t _newStt) = 0;

    virtual void onCabTryReadBp(uint32_t _id, int32_t _stt) = 0;

    virtual void onBpConnected(uint32_t _id, const std::string& _sn, const std::string& _version) = 0;

    virtual void onBpDisconnected(uint32_t _id, const std::string& _sn) = 0;

    virtual void onDeviceIsAssignedToBp(uint32_t _id,
                                        int32_t _isSuccess,
                                        const std::string& _devSn) = 0;
};

#endif //BSS_PROCESS_CABINETOBSERVERINTERFACE_H
