//
// Created by vnbk on 01/06/2023.
//

#ifndef CABMANAGEMENT_H
#define CABMANAGEMENT_H

#include <vector>
#include <string>
#include <cstring>
#include <mutex>
#include <condition_variable>

#include "bss_data_def.h"
#include "CabComInterface.h"
#include "CabObserverInterface.h"
#include "CabManagerInterface.h"
#include "BpManagerInterface.h"

#include "Timer.h"

class CabinetManager : public CabManagerInterface,
                        public BpObserverInterface{
public:
    static std::shared_ptr<CabinetManager> create(std::shared_ptr<CabComInterface> _if,
                                                  std::shared_ptr<BpManagerInterface> _bp_service,
                                                  uint32_t _number,
                                                  int32_t _cabTempOffset = 0,
                                                  int32_t _pogoTempOffset = 0);

    ~CabinetManager() override;

    bool isConnected(const uint32_t& _id) override;

    bool setUpgradingState(const uint32_t& _id, bool _isUpgrading) override;

    std::vector<uint32_t> getEmptyCabs() override;

    std::vector<uint32_t> getCabsWithBp() override;

    bool cabHaveBp(const uint32_t& _id) override;

    const std::vector<std::shared_ptr<Cab>> &getCabs() override;

    std::shared_ptr<Cab> &getCab(const uint32_t& _id) override;

    int32_t openDoor(const uint32_t& _id) override;

    int32_t ctlFan(const uint32_t& _id, uint8_t _value) override;

    int32_t ctlChargerSwitch(const uint32_t& _id, uint8_t _value) override;

    int32_t ctlNodeId(const uint32_t& _id, uint8_t _value) override;

    int32_t assignBp(const uint32_t& _id, int32_t _count) override;

    int32_t assignDeviceToBp(const uint32_t& _id, const std::string& _dev_sn) override;

    void onBpConnected(const uint32_t& _id, const std::string& _sn, const std::string& _version) override;

    void onBpDisconnected(const uint32_t& _id, const std::string& _sn) override;

    void onBpAssignStatus(const uint32_t& _id, int32_t _status) override;

    void onDeviceIsAssigned(const uint32_t& _id,
                            int32_t _status,
                            const std::string& _bp_sn,
                            const std::string& _dev_type,
                            const std::string& _dev_sn) override;

    void process() override;

private:
    CabinetManager(std::shared_ptr<CabComInterface> _if,
                   std::shared_ptr<BpManagerInterface> _bp_service,
                   uint32_t _number,
                   int32_t _cabTempOffset,
                   int32_t _pogoTempOffset);

    bool initialized();

    void updateData(BssCabData& _data, CabComInterface::CabComData& _comData);

    void getAssignBp();

    bool checkAssignCondition(const uint32_t& _id);

    void resetForceCabAssigning();

    std::shared_ptr<BpManagerInterface> m_bpManager;

    std::vector<std::shared_ptr<Cab>> m_cabs;
    uint32_t m_currentCab;
    uint32_t m_cabNumber;

    int32_t m_cabTempOffset;
    int32_t m_pogoTempOffset;

    int32_t m_bpAssignIndex;
    bool m_waitBpAssigning;

    bool m_forceAssigning;
    std::shared_ptr<Cab> m_cabAssigning;
    int32_t m_assigningCount;

    std::mutex m_lock;
    std::condition_variable m_trigger;
};

#endif //CABMANAGEMENT_H
