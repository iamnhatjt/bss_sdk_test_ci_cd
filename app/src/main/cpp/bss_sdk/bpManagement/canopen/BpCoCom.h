//
// Created by vnbk on 02/06/2023.
//

#ifndef BPMANAGEMENT_H
#define BPMANAGEMENT_H

#include <queue>

#include "BpComInterface.h"
#include "BpManagerInterface.h"
#include "BpAssignModule.h"
#include "NodeIdControllerInterface.h"

#include "Bp.h"
#include "CO.h"
#include "sm_co_if.h"
#include "Timer.h"

#include "CanMasterDef.h"

#define BP_NODEID_OFFSET_DEFAULT   5

class BpCoCom : public BpComInterface{
public:
    static std::shared_ptr<BpCoCom> create(const std::shared_ptr<NodeIdControllerInterface>& _node_if,
                                           sm_co_if_t* _co_if,
                                           int32_t _bp_number = BSS_CAB_NUMBER_DEFAULT,
                                           uint32_t _nodeIdOffset = BP_NODEID_OFFSET_DEFAULT,
                                           bool _autoAssign = false);

    static void bpManagementReceivedCallback(uint32_t _can_id, uint8_t* _data, void* _arg);

    ~BpCoCom() override;

    void addManager(std::shared_ptr<BpManagerInterface> _manager) override;

    bool sync(bool _enable) override;

    int32_t setCmd(const std::shared_ptr<Bp::BpCmd>& _cmd) override;

    int32_t assign(const std::shared_ptr<Bp>& _bp) override;

    int32_t resetCmd() override;

    bool isBusy() override;

    int32_t process() override;

private:
    BpCoCom(sm_co_if_t* _co_if,
            int32_t _bp_number,
            uint32_t _nodeIdOffset,
            bool _autoAssign);

    bool initialized(const std::shared_ptr<NodeIdControllerInterface>&);

    void pdoProcess(uint32_t _cobId, uint8_t* _data);

    int32_t cmdReadSn(const std::shared_ptr<Bp>& _bp);
    int32_t cmdReadVersion(const std::shared_ptr<Bp>& _bp);
    int32_t cmdCharging(const std::shared_ptr<Bp>& _bp);
    int32_t cmdDischarging(const std::shared_ptr<Bp>& _bp);
    int32_t cmdReboot(const std::shared_ptr<Bp>& _bp);
    int32_t cmdReadDeviceAssigned(const std::shared_ptr<Bp>& _bp);
    int32_t cmdWriteDeviceAssigned(const std::shared_ptr<Bp>& _bp, const std::string& _devSn);

    int32_t cmdHandle(const std::shared_ptr<Bp::BpCmd>& _cmd);
    int32_t cmdHandleResponse();

    void getAssignBp();

    uint32_t m_nodeIdOffset;
    uint32_t m_bpNumber;

    std::shared_ptr<BpAssignModule> m_assignModule;
    uint32_t m_bpAssignIndex;

    std::shared_ptr<Bp::BpCmd> m_currentCmd;

    bool m_autoAssign;
    bool m_sync;

    std::shared_ptr<BpManagerInterface> m_manager;

    CO* m_coDevice;
    sm_co_if_t* m_coInterface;
};


#endif //BPMANAGEMENT_H
