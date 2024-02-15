//
// Created by vnbk on 24/07/2023.
//

#ifndef BSS_SDK_CHARGERCOM_H
#define BSS_SDK_CHARGERCOM_H

#include "ChargerComInterface.h"

class ChargerCom : public ChargerComInterface{
public:
    ~ChargerCom() override;

    int32_t charged(const uint32_t& _id) override;

    int32_t setConfig(const uint32_t& _id,  bool _isAuto, const uint32_t& _cur, const uint32_t& _vol) override;

    int32_t discharged(const uint32_t& _id) override;

    int32_t getChargerVol(const uint32_t& _id) override;

    int32_t getChargerCur(const uint32_t& _id) override;

    int32_t getState(const uint32_t& _id) override;

    void process() override;
private:
};


#endif //BSS_SDK_CHARGERCOM_H
