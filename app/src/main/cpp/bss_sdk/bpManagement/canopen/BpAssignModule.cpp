//
// Created by vnbk on 10/07/2023.
//

#include "BpAssignModule.h"

#include <algorithm>
#include "Logger.h"
#include "app_co_init.h"

#define TAG "BpAssignModule"

static void bpAssignModuleCallback(BP_COMMAND _cmd, int32_t _isSuccess, const uint32_t& _id, void* _data, void* _arg){
    auto assignModule = (BpAssignModule*)_arg;
    if(!assignModule || !assignModule->isBpAssigning(_id)){
        LOG_ERROR(TAG, "Assigned Module is NULL or other Id: %d", _id);
        return;
    }
    if(_cmd == BP_COMMAND::BP_CMD_READ_SN){
        if(_isSuccess == BP_CMD_SUCCESS) {
            if (assignModule->getAssignState() == BP_ASSIGN_STATE::BP_ASSIGN_AUTHORIZING) {
                assignModule->setAssignState(BP_ASSIGN_STATE::BP_ASSIGNED);
                LOG_TRACE(TAG, "Assigning state is switched from BP_ASSIGN_AUTHORIZING to BP_ASSIGNED");
            }
        } else if(_isSuccess == BP_CMD_FAILURE){
            if (assignModule->getAssignState() == BP_ASSIGN_STATE::BP_ASSIGN_AUTHORIZING) {
                assignModule->setAssignState(BP_ASSIGN_STATE::BP_ASSIGNED);
                LOG_WARN(TAG, "Authorizing read SN FAILURE, Please tried again");
            }
        }
    }
}

BpAssignModule::BpAssignModule(std::shared_ptr<BpManagerInterface> _manager,
                               std::shared_ptr<NodeIdControllerInterface> _controller,
                               uint32_t _nodeIdOffset) : m_assigningTimeout(BP_ASSIGNING_TIMEOUT),
                                            m_authTimeout(BP_ASSIGNING_AUTH_TIMEOUT), m_retry(0),
                                            m_nodeIdOffset(_nodeIdOffset){
    m_manager = std::move(_manager);
    m_nodeIdController = std::move(_controller);
}

BpAssignModule::~BpAssignModule() {
    reset();
}

int32_t BpAssignModule::addManager(std::shared_ptr<BpManagerInterface> &_manager) {
    if(!_manager){
        return -1;
    }
    m_manager = _manager;
    return 0;
}

int32_t BpAssignModule::addBp(const std::shared_ptr<Bp>& _bp) {
    /// Lock
    std::lock_guard<std::mutex> lock(m_lock);

    if(m_assigningBp &&
        m_assigningBp->m_id == _bp->m_id &&
        m_assigningBp->m_assignStatus != BP_ASSIGN_STATE::BP_ASSIGN_FAILURE){
        LOG_DEBUG(TAG, "BP is assigning, please wait");
        return 0;
    }

    if(m_assigningBp){
        m_assigningBp->m_assignStatus = BP_ASSIGN_STATE::BP_ASSIGN_FAILURE;
    }

    reset();
    m_assigningBp = _bp;

    if(nodeIdSelect() < 0){
        m_manager->notifyAssigningState(m_assigningBp->m_id,
                                        BP_ASSIGN_STATE::BP_ASSIGN_FAILURE);
        m_assigningBp->reset();
        reset();
        return -1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    m_assigningTimeout.reset();
    m_assigningBp->m_assignStatus = BP_ASSIGN_STATE::BP_ASSIGN_WAIT_REQUEST;
    m_assigningBp->m_assigningRetried++;

    if(nodeIdDeselect() < 0){
        m_manager->notifyAssigningState(m_assigningBp->m_id,
                                        BP_ASSIGN_STATE::BP_ASSIGN_FAILURE);
        m_assigningBp->reset();
        reset();
        return -1;
    }

    LOG_TRACE(TAG,"Starting: Assigning state is switched to BP_ASSIGN_WAIT_REQUEST");

    return 0;
}

bool BpAssignModule::isBpAssigning(const uint32_t &_id) {
    return m_assigningBp && (m_assigningBp->m_id == (_id));
}

void BpAssignModule::recvAssignedData(const uint32_t &_id, const uint8_t *_data, int32_t _len) {
    if(_id == CAN_NODE_ID_ASSIGN_COBID){
        if(!m_assigningBp){
            LOG_ERROR(TAG, "No BP is assigning");
            return;
        }
        if(m_assigningBp->m_assignStatus == BP_ASSIGN_STATE::BP_ASSIGN_WAIT_REQUEST){
            sm_co_if_send(app_co_get_if(), CAN_NODE_ID_ASSIGN_COBID, nullptr, 0, 100);
            m_assigningBp->m_assignStatus = BP_ASSIGN_STATE::BP_ASSIGN_START;
            m_assigningBp->m_assigningRetried = 0;

            LOG_TRACE(TAG,"Assigning state is switched from BP_ASSIGN_WAIT_REQUEST to BP_ASSIGN_START");
        }
        else if(m_assigningBp->m_assignStatus == BP_ASSIGN_STATE::BP_ASSIGN_SLAVE_SELECT){
            m_assigningBp->m_assignStatus = BP_ASSIGN_STATE::BP_ASSIGN_SLAVE_SELECT_CONFIRM;
            LOG_TRACE(TAG,"Assigning state is switched from BP_ASSIGN_SLAVE_SELECT to BP_ASSIGN_SLAVE_SELECT_CONFIRM");
        }
        else if(m_assigningBp->m_assignStatus == BP_ASSIGN_STATE::BP_ASSIGN_WAIT_CONFIRM){
            m_assigningTimeout.reset();
            if (_data[0] != (m_assigningBp->m_id + m_nodeIdOffset))
                return;

            m_assigningBp->m_assignStatus = BP_ASSIGN_STATE::BP_ASSIGN_AUTHORIZING_START;
            m_authTimeout.reset(BP_ASSIGNING_AUTH_TIMEOUT);

            LOG_TRACE(TAG,"Assigning state is switched from BP_ASSIGN_WAIT_CONFIRM to BP_ASSIGN_AUTHORIZING_START");
        }
    }
}

BP_ASSIGN_STATE BpAssignModule::getAssignState() {
    return !m_assigningBp ? BP_ASSIGN_STATE::BP_ASSIGN_FAILURE : m_assigningBp->m_assignStatus;
}

void BpAssignModule::setAssignState(BP_ASSIGN_STATE _state) {
    if(m_assigningBp){
        m_assigningBp->m_assignStatus = _state;
    }
}

int32_t BpAssignModule::nodeIdDeselect(){
    int count = 0;
    while (count <= 2){
        int ret = m_nodeIdController->setNodeId(m_assigningBp->m_id,
                                                NodeIdControllerInterface::CANBUS_NODE_ID::NODE_ID_HIGH);
        if(ret > 0)
            return ret;
        count++;
    }
    LOG_ERROR(TAG, "DeSelect NODE ID FAILURE after %d time", count);
    return -1;
}

int32_t BpAssignModule::nodeIdSelect(){
    int count = 0;
    while (count <= 2){
        int ret = m_nodeIdController->setNodeId(m_assigningBp->m_id,
                                      NodeIdControllerInterface::CANBUS_NODE_ID::NODE_ID_LOW);
        if(ret > 0)
            return ret;
        count++;
    }
    LOG_ERROR(TAG, "Select NODE ID FAILURE after %d time", count);
    return -1;
}

void BpAssignModule::reset() {
    m_assigningBp = nullptr;
    m_assigningTimeout.reset();
    m_authTimeout.reset();
    m_retry = 0;
}

int32_t BpAssignModule::process() {
    std::lock_guard<std::mutex> lockGuard(m_lock);
    if(!m_assigningBp){
        return -1;
    }
    std::string sn;
    uint8_t nodeId = m_assigningBp->m_id + m_nodeIdOffset;
    switch (m_assigningBp->m_assignStatus) {
        case BP_ASSIGN_STATE::BP_ASSIGN_WAIT_REQUEST:
        case BP_ASSIGN_STATE::BP_ASSIGN_SLAVE_SELECT:
        case BP_ASSIGN_STATE::BP_ASSIGN_WAIT_CONFIRM:
            if (!m_assigningTimeout.getRemainTime()) {
                LOG_TRACE(TAG, "Bp %d Timeout Assigning progress at step %s",
                          m_assigningBp->m_id,
                          convertBpAssignStateToString(m_assigningBp->m_assignStatus).c_str());
                m_assigningBp->m_assignStatus = BP_ASSIGN_STATE::BP_ASSIGN_FAILURE;
            }
            break;
        case BP_ASSIGN_STATE::BP_ASSIGN_START:
            if(nodeIdSelect() < 0){
                m_manager->notifyAssigningState(m_assigningBp->m_id,
                                                BP_ASSIGN_STATE::BP_ASSIGN_FAILURE);
                m_assigningBp->reset();
                reset();
                break;
            }

            m_assigningBp->m_assignStatus = BP_ASSIGN_STATE::BP_ASSIGN_SLAVE_SELECT;
            m_assigningTimeout.reset();
            break;
        case BP_ASSIGN_STATE::BP_ASSIGN_SLAVE_SELECT_CONFIRM:
            sm_co_if_send(app_co_get_if(), CAN_NODE_ID_ASSIGN_COBID,
                          (const uint8_t*)&nodeId,
                          1,
                          100);
            m_assigningBp->m_assignStatus = BP_ASSIGN_STATE::BP_ASSIGN_WAIT_CONFIRM;
            m_assigningTimeout.reset();
            LOG_TRACE(TAG,"Assigning state is switched to WAIT_CONFIRM");
            break;

        case BP_ASSIGN_STATE::BP_ASSIGN_AUTHORIZING_START:
            if(nodeIdDeselect() < 0){
                m_manager->notifyAssigningState(m_assigningBp->m_id,
                                                BP_ASSIGN_STATE::BP_ASSIGN_FAILURE);
                m_assigningBp->reset();
                reset();
                break;
            }

            m_manager->setCmd(m_assigningBp->m_id,
                              BP_COMMAND::BP_CMD_READ_SN,
                              nullptr,
                              bpAssignModuleCallback,
                              this,
                              true);

            m_assigningBp->m_assignStatus = BP_ASSIGN_STATE::BP_ASSIGN_AUTHORIZING;
            m_authTimeout.reset();
            LOG_TRACE(TAG,"Assigning state is switched from BP_ASSIGN_AUTHORIZING_START to BP_ASSIGN_AUTHORIZING");
            break;
        case BP_ASSIGN_STATE::BP_ASSIGN_AUTHORIZING:
            if(!m_authTimeout.getRemainTime()){
                m_assigningBp->m_assignStatus = BP_ASSIGN_STATE::BP_ASSIGN_FAILURE;
                LOG_ERROR(TAG, "Assigning progress FAILURE, reason: Authorizing TIMEOUT");
            }
            break;
        case BP_ASSIGN_STATE::BP_ASSIGNED:
            sn = (char*)m_assigningBp->m_sn;
            std::reverse(sn.begin(), sn.end());

            if(sn.empty()){
                if(m_retry >= BP_ASSIGNING_RETRY_MAX){
                    LOG_ERROR(TAG, "Retried % time, but Could NOT read BP serial. Authorizing FAILURE");
                    m_assigningBp->m_assignStatus = BP_ASSIGN_STATE::BP_ASSIGN_FAILURE;
                    return 0;
                }
                m_assigningBp->m_assignStatus = BP_ASSIGN_STATE::BP_ASSIGN_AUTHORIZING_START;
                m_retry++;
                LOG_WARN(TAG, "Retried: %d time Authorizing - Get SN with BP: %d", m_retry, m_assigningBp->m_id);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                return 0;
            }

            if(!verifyAsciiString(sn, m_assigningBp->m_info.m_sn)){
                LOG_WARN(TAG, "BP Serial Number is INVALID");
                if(m_assigningBp->m_info.m_sn.empty()){
                    m_assigningBp->m_info.m_sn = "1N00_XXXX";
                }
            }

            LOG_DEBUG(TAG, "Assigning SUCCESS, BP info node id: %d, sn: %s",
                      m_assigningBp->m_id,
                      m_assigningBp->m_info.m_sn.c_str(),
                      m_assigningBp->m_version);

            m_manager->notifyAssigningState(m_assigningBp->m_id,
                                            BP_ASSIGN_STATE::BP_ASSIGNED,
                                            m_assigningBp->m_info.m_sn);

            m_assigningBp->m_timeout.reset();
            reset();
            break;
        case BP_ASSIGN_STATE::BP_ASSIGN_FAILURE:
            m_manager->notifyAssigningState(m_assigningBp->m_id,
                                            BP_ASSIGN_STATE::BP_ASSIGN_FAILURE);

            m_assigningBp->reset();
            reset();
            return -1;
    }
    return 0;
}
