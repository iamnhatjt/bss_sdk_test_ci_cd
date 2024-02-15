//
// Created by vnbk on 24/07/2023.
//

#ifndef BSS_SDK_CHARGERCOMINTERFACE_H
#define BSS_SDK_CHARGERCOMINTERFACE_H

#include <memory>

class ChargerComInterface {
public:
    virtual ~ChargerComInterface() = 0;

    virtual int32_t charged(const uint32_t& _id) = 0;

    virtual int32_t setConfig(const uint32_t& _id, bool _isAuto, const uint32_t& _cur, const uint32_t& _vol) = 0;

    virtual int32_t discharged(const uint32_t& _id) = 0;

    virtual int32_t getChargerVol(const uint32_t& _id) = 0;

    virtual int32_t getChargerCur(const uint32_t& _id) = 0;

    virtual int32_t getState(const uint32_t& _id) = 0;

    virtual void process() = 0;
};

#endif //BSS_SDK_CHARGERCOMINTERFACE_H
