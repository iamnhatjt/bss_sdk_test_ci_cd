//
// Created by vnbk on 10/08/2023.
//

#ifndef BSS_SDK_BPMANAGEMENT_H
#define BSS_SDK_BPMANAGEMENT_H

#include <queue>
#include <mutex>

#include "BpManagerInterface.h"
#include "BpComInterface.h"

#include "Timer.h"

class BpManager : public BpManagerInterface{
public:
    static std::shared_ptr<BpManager> create(std::shared_ptr<BpComInterface> _bpCom, uint32_t _bpNumber);

    static void bpCmdHandleCallback(BP_COMMAND _cmd, int32_t _isSuccess, const uint32_t& _id, void* _data, void* _arg);

    int32_t setConfiguration(const uint32_t& _id) override;
    int32_t getConfiguration(const uint32_t& _id) override;

    const BpData& getData(const uint32_t& _id) override;
    std::shared_ptr<Bp> getBp(const uint32_t& _id) override;

    int32_t assign(const uint32_t& _id) override;
    int32_t getAssigningStatus(const uint32_t& _id) override;
    int32_t getAssigningRetriedNumber(const uint32_t& _id) override;

    int32_t resetBp(const uint32_t& _id) override;

    int32_t getDeviceList(std::vector<BpData>& _list) override;

    int32_t setCmd(const uint32_t& _id,
                           BP_COMMAND _cmd,
                           void* _data,
                           BpCmdCallback_t _fn,
                           void* _arg,
                           bool _force = false) override;

    int32_t resetCmd() override;

    int32_t resetCurrentCmd() override;

    int32_t assignDeviceToBp(const uint32_t& _id,
                                 const std::string& _dev_sn) override;

    int32_t process() override;
    
private:
    BpManager(std::shared_ptr<BpComInterface> _bpCom, uint32_t _bp_number);

    bool initialized();

    uint32_t m_bpNumber;
    std::vector<std::shared_ptr<Bp>> m_bps;

    std::queue<std::shared_ptr<Bp::BpCmd>> m_cmdQueue;
    std::shared_ptr<Bp::BpCmd> m_currentCmd;

    std::mutex m_queueLock;
    std::mutex m_lock;
};


#endif //BSS_SDK_BPMANAGEMENT_H
