//
// Created by vnbk on 24/07/2023.
//

#include "PowerManager.h"

#include <utility>
#include "Logger.h"

#define TAG "PowerManager"

std::shared_ptr<PowerManager> PowerManager::create(std::shared_ptr<EMeterManagerInterface> _eMeter,
                                                   std::shared_ptr<ChargerManagerInterface> _chargerManager,
                                                   std::shared_ptr<BpManagerInterface> _bpManager,
                                                   std::shared_ptr<CabManagerInterface> _cabManager){
    auto powerManager = std::shared_ptr<PowerManager>(new PowerManager(std::move(_eMeter),
                                                                       std::move(_chargerManager),
                                                                       std::move(_bpManager),
                                                                       std::move(_cabManager)));
    if(powerManager->initialized()){
        LOG_INFO(TAG, "Create PowerManager SUCCESS");
        return powerManager;
    }
    return nullptr;
}

PowerManager::~PowerManager() noexcept {
    m_eMeterCom.reset();
}

PowerManager::PowerManager(std::shared_ptr<EMeterManagerInterface> _eMeter,
                           std::shared_ptr<ChargerManagerInterface> _chargerManager,
                           std::shared_ptr<BpManagerInterface> _bpManager,
                           std::shared_ptr<CabManagerInterface> _cabManager) : m_eMeterCom(std::move(_eMeter)),
                                                                                m_chargerManager(std::move(_chargerManager)),
                                                                                m_bpManager(std::move(_bpManager)),
                                                                                m_cabManager(std::move(_cabManager)),
                                                                                m_chargerNumber(BSS_CHARGER_NUMBER_DEFAULT){

}

bool PowerManager::initialized() {
    if(!m_eMeterCom){
        LOG_ERROR(TAG, "Missing EMeterCom parameters");
        return false;
    }

    return true;
}

EMeterManagerInterface::EMeterInfo& PowerManager::getEMeter(){
    return m_eMeterInfo;
}

void PowerManager::process() {
    m_eMeterCom->process();
}