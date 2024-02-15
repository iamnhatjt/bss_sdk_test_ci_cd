//
// Created by vnbk on 01/06/2023.
//

#ifndef BPMANAGERINTERFACE_H
#define BPMANAGERINTERFACE_H

#include <utility>
#include <vector>
#include <memory>

#include "NodeIdControllerInterface.h"
#include "BpObserverInterface.h"
#include "Bp.h"

#include "bss_data_def.h"

class BpComInterface;

class BpManagerInterface{
public:
    BpManagerInterface(std::shared_ptr<BpComInterface> _if) : m_bpInterface(std::move(_if)){}
    virtual ~BpManagerInterface() = default;

    int32_t addObserver(const std::shared_ptr<BpObserverInterface>& _observer){
        if(!_observer){
            return -1;
        }
        this->m_observer = _observer;
        return 0;
    }

    const std::shared_ptr<BpComInterface>& getInterface(){
        return m_bpInterface;
    }

    virtual int32_t setConfiguration(const uint32_t& _id) = 0;
    virtual int32_t getConfiguration(const uint32_t& _id) = 0;

    virtual const BpData& getData(const uint32_t& _id) = 0;
    virtual std::shared_ptr<Bp> getBp(const uint32_t& _id) = 0;

    virtual int32_t assign(const uint32_t& _id) = 0;
    virtual int32_t getAssigningStatus(const uint32_t& _id) = 0;
    virtual int32_t getAssigningRetriedNumber(const uint32_t& _id) = 0;

    virtual int32_t resetBp(const uint32_t& _id) = 0;

    virtual int32_t getDeviceList(std::vector<BpData>& _list) = 0;

    virtual int32_t setCmd(const uint32_t& _id,
                           BP_COMMAND _cmd,
                           void* _data,
                           BpCmdCallback_t _fn,
                           void* _arg,
                           bool _force = false) = 0;

    virtual int32_t resetCmd() = 0;

    virtual int32_t resetCurrentCmd() = 0;

    virtual int32_t assignDeviceToBp(const uint32_t& _id,
                                 const std::string& _dev_sn) = 0;

    virtual int32_t process() = 0;

    void notifyAssigningState(const uint32_t& _id,
                              BP_ASSIGN_STATE _state,
                              const std::string& _sn = "",
                              const std::string& _version = ""){
        if(!m_observer){
            return;
        }

        m_observer->onBpAssignStatus(_id, (int32_t)_state);
        if(_state == BP_ASSIGN_STATE::BP_ASSIGNED && !_sn.empty()){
            m_observer->onBpConnected(_id, _sn, _version);
        }
    }

protected:
    std::shared_ptr<BpObserverInterface> m_observer;
    std::shared_ptr<BpComInterface> m_bpInterface;
};

#endif //BPMANAGERINTERFACE_H
