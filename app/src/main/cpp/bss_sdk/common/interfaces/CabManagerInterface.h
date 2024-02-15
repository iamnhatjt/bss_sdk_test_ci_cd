//
// Created by vnbk on 24/07/2023.
//

#ifndef BSS_SDK_CABMANAGERINTERFACE_H
#define BSS_SDK_CABMANAGERINTERFACE_H

#include <memory>
#include <utility>
#include <vector>

#include "bss_data_def.h"

#include "CabObserverInterface.h"
#include "CabComInterface.h"
#include "Timer.h"

#define CAB_RECONNECT_TIME_UPPER          (1000*1000) //ms
#define CAB_RECONNECT_TIME_LOWER          (500*1000) //ms

#define CAB_RECONNECT_RETRY_COUNT   5
#define CAB_CHECK_BP_CONNECTED_TIME (60*1000) //ms

struct Cab {
    uint8_t m_id;
    BssCabData m_data;

    bool m_connectionAvailable;
    uint8_t m_prevDoorStatus;
    uint8_t m_retry;
    WaitTimer m_reconnectTime;
    WaitTimer m_checkBpConnected;

    explicit Cab(uint8_t _id) : m_reconnectTime(CAB_RECONNECT_TIME_UPPER),
                                m_checkBpConnected(CAB_CHECK_BP_CONNECTED_TIME){
        m_id = _id;
        m_data.m_isConnected = false;
        m_prevDoorStatus = CAB_DOOR_CLOSE_STATUS;
        m_retry = 0;
        reset();
    };

    Cab& operator=(const Cab& _other){
        this->m_id = _other.m_id;
        this->m_data = _other.m_data;
        this->m_prevDoorStatus = _other.m_prevDoorStatus;
        this->m_reconnectTime = _other.m_reconnectTime;
        this->m_retry = _other.m_retry;
        this->m_checkBpConnected = _other.m_checkBpConnected;
        this->m_connectionAvailable = _other.m_connectionAvailable;
        return *this;
    }

    void reset(){
        m_connectionAvailable = false;
        m_checkBpConnected.reset();
        m_reconnectTime.reset();
        m_retry = 0;
        m_prevDoorStatus = CAB_DOOR_CLOSE_STATUS;
        m_data.m_bpSoc = 0;
        m_data.m_doorStt = CAB_DOOR_CLOSE_STATUS;
        m_data.m_fanStt = CAB_FAN_TURN_OFF_STATUS;
        m_data.m_temp = 0;
        m_data.m_chargerSwitchStt = CAB_CHARGER_TURN_OFF_STATUS;
        m_data.m_bpSn.clear();
        m_data.m_pogoPinTemp[0] = 0;
        m_data.m_pogoPinTemp[1] = 0;
        m_data.m_pogoPinTemp[2] = 0;
        m_data.m_pogoPinTemp[3] = 0;
        m_data.m_isConnected = false;
        m_data.m_isUpgrading = false;
        m_data.m_isCharged = false;
    }
};

class CabManagerInterface{
public:
    explicit CabManagerInterface(std::shared_ptr<CabComInterface> _interface,
                        std::shared_ptr<CabObserverInterface> _observer = nullptr) :
                        m_interface(std::move(_interface)),
                        m_observer(std::move(_observer)){

    }
    virtual ~CabManagerInterface() = default;

    int32_t addObserver(const std::shared_ptr<CabObserverInterface>& _observer){
        if(_observer){
            this->m_observer = _observer;
            return 0;
        }
        return -1;
    }

    void removeObserver(){
        this->m_observer = nullptr;
    }

    virtual bool isConnected(const uint32_t& _id) = 0;

    virtual bool setUpgradingState(const uint32_t& _id, bool _isUpgrading) = 0;

    virtual std::vector<uint32_t> getEmptyCabs() = 0;

    virtual std::vector<uint32_t> getCabsWithBp() = 0;

    virtual bool cabHaveBp(const uint32_t& _id) = 0;

    virtual const std::vector<std::shared_ptr<Cab>> &getCabs() = 0;

    virtual std::shared_ptr<Cab> &getCab(const uint32_t& _id) = 0;

    virtual int32_t openDoor(const uint32_t& _id) = 0;

    virtual int32_t ctlFan(const uint32_t& _id, uint8_t _value) = 0;

    virtual int32_t ctlChargerSwitch(const uint32_t& _id, uint8_t _value) = 0;

    virtual int32_t ctlNodeId(const uint32_t& _id, uint8_t _value) = 0;

    virtual int32_t assignBp(const uint32_t& _id, int32_t _count) = 0;

    virtual int32_t assignDeviceToBp(const uint32_t& _id, const std::string& _dev_sn) = 0;

    virtual void process() = 0;

protected:
    std::shared_ptr<CabObserverInterface> m_observer;
    std::shared_ptr<CabComInterface> m_interface;
};

#endif //BSS_SDK_CABMANAGERINTERFACE_H
