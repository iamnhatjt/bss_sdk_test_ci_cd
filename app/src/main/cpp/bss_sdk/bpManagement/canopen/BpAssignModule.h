//
// Created by vnbk on 10/07/2023.
//

#ifndef BSS_SDK_BPASSIGNMODULE_H
#define BSS_SDK_BPASSIGNMODULE_H

#include <mutex>

#include "BpManagerInterface.h"
#include "NodeIdControllerInterface.h"
#include "Bp.h"

#define BP_ASSIGNING_TIMEOUT        2000 ///3000
#define BP_ASSIGNING_AUTH_TIMEOUT   6000//3000
#define BP_ASSIGNING_RETRY_MAX      3

#define CAN_NODE_ID_ASSIGN_COBID	0x70

class BpAssignModule {
public:
    explicit BpAssignModule(std::shared_ptr<BpManagerInterface> _manager,
                            std::shared_ptr<NodeIdControllerInterface> _nodeIdController,
                            uint32_t _nodeIdOffset);
    ~BpAssignModule();

    int32_t addManager(std::shared_ptr<BpManagerInterface>& _manager);

    int32_t addBp(const std::shared_ptr<Bp>& _bp);
    bool isBpAssigning(const uint32_t& _id);

    void recvAssignedData(const uint32_t& _id, const uint8_t* _data, int32_t _len);
    BP_ASSIGN_STATE getAssignState();
    void setAssignState(BP_ASSIGN_STATE _state);

    int32_t process();
private:
    int32_t nodeIdDeselect();
    int32_t nodeIdSelect();

    void reset();

    std::shared_ptr<Bp> m_assigningBp;
    WaitTimer m_assigningTimeout;
    WaitTimer m_authTimeout;
    int32_t  m_retry;

    uint32_t m_nodeIdOffset;

    std::shared_ptr<BpManagerInterface> m_manager;
    std::shared_ptr<NodeIdControllerInterface> m_nodeIdController;

    std::mutex m_lock;
};


#endif //BSS_SDK_BPASSIGNMODULE_H
