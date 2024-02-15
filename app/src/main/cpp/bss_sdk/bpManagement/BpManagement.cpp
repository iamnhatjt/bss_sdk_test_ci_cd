//
// Created by vnbk on 10/08/2023.
//
#include <algorithm>
#include "BpManagement.h"

#include "Logger.h"

#define TAG "BpManager"

void BpManager::bpCmdHandleCallback(BP_COMMAND _cmd, int32_t _isSuccess, const uint32_t& _id, void* _data, void* _arg){
    auto bpManagement = (BpManager*)_arg;
    if(!bpManagement){
        LOG_ERROR(TAG, "BpManager is NULL");
        return;
    }

    auto bp = bpManagement->m_bps[_id];
    if(_cmd == BP_COMMAND::BP_CMD_WRITE_ASSIGNED_DEVICE){
        if(bpManagement->m_observer){
            std::string device = (char*)_data;
            if(_isSuccess == BP_CMD_SUCCESS){
                bp->m_info.m_assignedSn = device;

                bpManagement->m_observer->onDeviceIsAssigned(_id,
                                                             BSS_RET_SUCCESS,
                                                             bp->m_info.m_sn,
                                                             "",
                                                             device);
            }else{
                bpManagement->m_observer->onDeviceIsAssigned(_id,
                                                             BSS_RET_FAILURE,
                                                             bp->m_info.m_sn,
                                                             "",
                                                             device);
            }
        }
    }

    if(_cmd == BP_COMMAND::BP_CMD_READ_VERSION){
        if(_isSuccess == BP_CMD_SUCCESS){
            auto data = (uint8_t*)_data;

            std::string version;
            for(int i = 0; i < 3; ++i){
                version.append(std::to_string(data[2-i]));
                version.append(".");
            }

            version = version.substr(0,version.size()-1);

            if(!verifyAsciiString(version, bp->m_info.m_version)){
                LOG_ERROR(TAG, "Version value is INVALID: %s", version.c_str());
                if(bp->m_info.m_version.empty())
                    bp->m_info.m_version = "INVALID";
            }else{
//                bp->m_info.m_version = version;
                LOG_DEBUG(TAG, "BP %s version: %s", bp->m_info.m_sn.c_str(),  bp->m_info.m_version.c_str());
            }

            bp->m_initStep++;
        }
        bp->m_waitingCmd = false;
    }

    if(_cmd == BP_COMMAND::BP_CMD_READ_ASSIGNED_DEVICE){
        if(_isSuccess == BP_CMD_SUCCESS) {
            std::string dev_sn;
            if(!verifyAsciiString((char*)_data, dev_sn)){
                LOG_ERROR(TAG, "Device SN of BP is INVALID: %s", (char*)_data);
                if(dev_sn.empty()){
                    bp->m_info.m_assignedSn = "INVALID";
                }
            }else{
                bp->m_info.m_assignedSn = dev_sn;
                LOG_DEBUG(TAG, "The device %s is assigned to BP %s",
                          bp->m_info.m_assignedSn.c_str(),
                          bp->m_info.m_sn.c_str());
            }
            bp->m_info.m_assignedSn = dev_sn;

            bp->m_initStep++;
        }
        bp->m_waitingCmd = false;
    }

    if(_cmd == BP_COMMAND::BP_CMD_REBOOT && _isSuccess == BP_CMD_SUCCESS){
        LOG_INFO(TAG, "BP reboot SUCCESS");
    }

    if(!bpManagement->m_currentCmd){
        return;
    }

    if((bpManagement->m_currentCmd->m_bp->m_id == _id) && (bpManagement->m_currentCmd->m_cmd == _cmd)){
        bpManagement->m_currentCmd.reset();
    }
}

std::shared_ptr<BpManager> BpManager::create(std::shared_ptr<BpComInterface> _bpCom, uint32_t _bpNumber) {
    auto bpManager = std::shared_ptr<BpManager>(new BpManager(std::move(_bpCom), _bpNumber));
    if(bpManager->initialized()){
        LOG_INFO(TAG, "Create BpManager SUCCESS");
        return bpManager;
    }
    return nullptr;
}

BpManager::BpManager(std::shared_ptr<BpComInterface> _bpCom,
                 uint32_t _bp_number) : BpManagerInterface(std::move(_bpCom)),
                                        m_bpNumber(_bp_number),
                                        m_currentCmd(nullptr){

}

bool BpManager::initialized() {
    if(!m_bpInterface || !m_bpNumber){
        return false;
    }
    resetCmd();

    m_bps.clear();

    for (int i = 0; i < m_bpNumber; ++i) {
        auto bp = std::make_shared<Bp>(i);
        m_bps.push_back(bp);
    }

    return true;
}

int32_t BpManager::setConfiguration(const uint32_t& _id) {
    return 0;
}

int32_t BpManager::getConfiguration(const uint32_t& _id) {
    return 0;
}

const BpData& BpManager::getData(const uint32_t& _id) {
    return m_bps[_id]->m_info;
}

std::shared_ptr<Bp> BpManager::getBp(const uint32_t &_id) {
    return m_bps[_id];
}

int32_t BpManager::assign(const uint32_t& _id) {
    if(m_bps[_id]->m_assignStatus == BP_ASSIGN_STATE::BP_ASSIGNED){
        notifyAssigningState(_id,BP_ASSIGN_STATE::BP_ASSIGNED,
                             m_bps[_id]->m_info.m_sn,
                             m_bps[_id]->m_info.m_version);
    }
    m_bpInterface->assign(m_bps[_id]);
    return 0;
}

int32_t BpManager::getAssigningStatus(const uint32_t& _id) {
    return (int32_t)m_bps[_id]->m_assignStatus;
}

int32_t BpManager::getAssigningRetriedNumber(const uint32_t &_id) {
    return (int32_t)m_bps[_id]->m_assigningRetried;
}

int32_t BpManager::resetBp(const uint32_t &_id) {
    std::lock_guard<std::mutex> lockGuard(m_lock);
    if(m_bps[_id]->m_assignStatus == BP_ASSIGN_STATE::BP_ASSIGNED) {
        m_observer->onBpDisconnected(m_bps[_id]->m_id, m_bps[_id]->m_info.m_sn);
    }
    m_bps[_id]->reset();
    return 0;
}

int32_t BpManager::getDeviceList(std::vector<BpData>& _list) {
    for (const auto& item : m_bps) {
        if(item->m_assignStatus == BP_ASSIGN_STATE::BP_ASSIGNED && !item->m_info.m_sn.empty()){
            _list.push_back(item->m_info);
        }
    }
    return (int32_t)_list.size();
}

int32_t BpManager::setCmd(const uint32_t& _id,
                        BP_COMMAND _cmd,
                        void* _data,
                        BpCmdCallback_t _fn,
                        void* _arg,
                        bool _force) {
    if(_cmd >= BP_COMMAND::BP_CMD_NUMBER)
        return -1;
    auto bp = m_bps[_id];

    std::lock_guard<std::mutex> lockGuard(m_queueLock);

    if(_force){
        if(m_currentCmd){
            m_cmdQueue.push(m_currentCmd);
        }

        auto cmd = std::make_shared<Bp::BpCmd>(bp, _cmd, _data, _fn, _arg);
        m_currentCmd = cmd;

        m_bpInterface->resetCmd();
        m_bpInterface->setCmd(m_currentCmd);
        return 0;
    }

    LOG_DEBUG(TAG, "Push Bp %d command %s to queue, waiting for process", _id, convertBpCmdToString(_cmd).c_str());

    auto cmd = std::make_shared<Bp::BpCmd>(bp, _cmd, _data, _fn, _arg);
    m_cmdQueue.push(cmd);

    return 0;
}

int32_t BpManager::resetCmd() {
    std::lock_guard<std::mutex> lockGuard(m_queueLock);
    while (!m_cmdQueue.empty()){
        m_cmdQueue.pop();
    }
    if(m_currentCmd && m_currentCmd->m_callback){
        m_currentCmd->m_callback(m_currentCmd->m_cmd,
                                 BP_CMD_FAILURE,
                                 m_currentCmd->m_bp->m_id,
                                 m_currentCmd->m_data,
                                 m_currentCmd->m_arg);
    }
    m_currentCmd.reset();

    return 0;
}

int32_t BpManager::resetCurrentCmd() {
    m_currentCmd.reset();
    return 0;
}

int32_t BpManager::assignDeviceToBp(const uint32_t& _id,
                                  const std::string& _dev_sn) {
    std::string value;
    if(!verifyAsciiString(_dev_sn, value)){
        LOG_ERROR(TAG, "Device SN that is assigned to BP INVALID");
        return -1;
    }

    auto bp = m_bps[_id];
    if(bp->m_assignStatus == BP_ASSIGN_STATE::BP_ASSIGNED){
        memcpy(bp->m_assignedSn, _dev_sn.c_str(), _dev_sn.size());
        bp->m_assignedSn[_dev_sn.size()] = '\0';
        LOG_DEBUG(TAG, "BP CMD: Write device %s sn to BP %d", _dev_sn.c_str(), _id);
        return setCmd(_id,
                      BP_COMMAND::BP_CMD_WRITE_ASSIGNED_DEVICE,
                      bp->m_assignedSn,
                      BpManager::bpCmdHandleCallback,
                      this);
    }
    LOG_ERROR(TAG, "Write device %s sn to BP %d FAILURE,Reason: BP is NOT assigned", _dev_sn.c_str(), _id);
    return -1;
}

int32_t BpManager::process() {
    m_bpInterface->process();

    for(const auto& item : m_bps){
        if(item->m_assignStatus == BP_ASSIGN_STATE::BP_ASSIGNED){
            if(item->m_info.m_version.empty() && !item->m_waitingCmd && (item->m_initStep == BP_INITIALIZED_STEP_0)){
                setCmd(item->m_id,
                       BP_COMMAND::BP_CMD_READ_VERSION,
                       item->m_version,
                       bpCmdHandleCallback,
                       this);
                item->m_waitingCmd = true;
            }

            if(!item->m_waitingCmd && (item->m_initStep == BP_INITIALIZED_STEP_1)){
                setCmd(item->m_id,
                       BP_COMMAND::BP_CMD_READ_ASSIGNED_DEVICE,
                       item->m_assignedSn,
                       bpCmdHandleCallback,
                       this);
                item->m_waitingCmd = true;
            }

            if(!item->m_timeout.getRemainTime()) {
                m_lock.lock();
                if (m_observer)
                    m_observer->onBpDisconnected(item->m_id, item->m_info.m_sn);

                item->reset();
                m_lock.unlock();
            }
        }
    }

    if(!m_cmdQueue.empty() && !m_currentCmd && !m_bpInterface->isBusy()){
        m_queueLock.lock();
        auto cmd = m_cmdQueue.front();
        m_cmdQueue.pop();
        m_queueLock.unlock();

        m_currentCmd = cmd;
        m_currentCmd->m_timeout.reset();

        m_bpInterface->setCmd(m_currentCmd);

        LOG_DEBUG(TAG, "Start to handle CMD: %s", convertBpCmdToString(cmd->m_cmd).c_str());
    }

    if(m_currentCmd && !m_currentCmd->m_timeout.getRemainTime()){
        LOG_ERROR(TAG, "Bp command FAILURE: %s, Reason TIMEOUT", convertBpCmdToString(m_currentCmd->m_cmd).c_str());
        if(m_currentCmd->m_callback){
            m_currentCmd->m_callback(m_currentCmd->m_cmd,
                                     BP_CMD_FAILURE,
                                     m_currentCmd->m_bp->m_id,
                                     m_currentCmd->m_data,
                                     m_currentCmd->m_arg);
        }
        m_bpInterface->resetCmd();
        if(m_currentCmd) {
            m_currentCmd.reset();
        }
        return 0;
    }
    return 0;
}
