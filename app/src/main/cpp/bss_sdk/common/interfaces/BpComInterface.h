//
// Created by vnbk on 10/08/2023.
//

#ifndef BSS_SDK_BPCOMINTERFACE_H
#define BSS_SDK_BPCOMINTERFACE_H

#include <memory>
#include <utility>

#include "BpManagerInterface.h"

class BpComInterface{
public:
    virtual ~BpComInterface() = default;

    virtual void addManager(std::shared_ptr<BpManagerInterface> _manager) = 0;

    virtual bool sync(bool _enable) = 0;

    virtual int32_t assign(const std::shared_ptr<Bp>& _bp) = 0;

    virtual int32_t setCmd(const std::shared_ptr<Bp::BpCmd>& _cmd) = 0;

    virtual int32_t resetCmd() = 0;

    virtual bool isBusy() = 0;

    virtual int32_t process() = 0;
};

#endif //BSS_SDK_BPCOMINTERFACE_H
