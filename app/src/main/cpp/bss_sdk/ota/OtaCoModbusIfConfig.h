//
// Created by vnbk on 10/10/2023.
//

#ifndef BSS_SDK_OTAIFCONFIG_H
#define BSS_SDK_OTAIFCONFIG_H

#include "ModbusMasterInterface.h"
#include "BssModbusDefine.h"

#include "app_co_init.h"
#include "sm_co_if.h"

class OtaCoModbusIfConfig{
public:
    explicit OtaCoModbusIfConfig(std::shared_ptr<ModbusMasterInterface> &_mbMaster) : m_mbMaster(_mbMaster),
                                                                              m_coModbusIf(nullptr) {
    }

    bool config(){
        if(!m_mbMaster){
            return false;
        }

        m_coModbusIf = sm_co_if_create_default(CO_MODBUS_CANBUS_IF, nullptr, -1, m_mbMaster.get());
        if(!m_coModbusIf){
            return false;
        }

        app_co_init(m_coModbusIf, nullptr, nullptr);

        app_co_sync(false);

        return true;
    }

    void process(){
        app_co_process();
    }

private:
    std::shared_ptr<ModbusMasterInterface> m_mbMaster;

    sm_co_if_t* m_coModbusIf;
};

#endif //BSS_SDK_OTAIFCONFIG_H
