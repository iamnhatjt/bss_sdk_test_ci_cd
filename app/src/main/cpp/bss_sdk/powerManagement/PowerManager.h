//
// Created by vnbk on 24/07/2023.
//

#ifndef BSS_SDK_POWERMANAGER_H
#define BSS_SDK_POWERMANAGER_H

#include "bss_data_def.h"

#include "EMeterManagerInterface.h"
#include "ChargerManagerInterface.h"
#include "BpManagerInterface.h"
#include "CabManagerInterface.h"

class PowerManager{
public:
    static std::shared_ptr<PowerManager> create(std::shared_ptr<EMeterManagerInterface> _eMeter,
                                                std::shared_ptr<ChargerManagerInterface> _chargerManager,
                                                std::shared_ptr<BpManagerInterface> _bpManager,
                                                std::shared_ptr<CabManagerInterface> _cabManager);

    ~PowerManager();

    EMeterManagerInterface::EMeterInfo& getEMeter();

    void process();
private:
    PowerManager(std::shared_ptr<EMeterManagerInterface> _eMeter,
                 std::shared_ptr<ChargerManagerInterface> _chargerManager,
                 std::shared_ptr<BpManagerInterface> _bpManager,
                 std::shared_ptr<CabManagerInterface> _cabManager);

    bool initialized();

    uint32_t m_chargerNumber;

    std::shared_ptr<EMeterManagerInterface> m_eMeterCom;
    EMeterManagerInterface::EMeterInfo m_eMeterInfo;

    std::shared_ptr<BpManagerInterface> m_bpManager;
    std::shared_ptr<CabManagerInterface> m_cabManager;
    std::shared_ptr<ChargerManagerInterface> m_chargerManager;
};


#endif //BSS_SDK_POWERMANAGER_H
