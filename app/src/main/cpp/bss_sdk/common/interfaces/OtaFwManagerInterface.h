//
// Created by vnbk on 12/09/2023.
//

#ifndef BSS_SDK_OTAFWMANAGERINTERFACE_H
#define BSS_SDK_OTAFWMANAGERINTERFACE_H

#include <memory>
#include <vector>

class OtaFwManagerInterface{
public:
    virtual ~OtaFwManagerInterface() = default;

    virtual void loadFw(const uint8_t& _devType) = 0;

    virtual int32_t addNewFw(const uint8_t& _devType, const std::string& _version, const std::string& _path) = 0;

    virtual int32_t removeFw(const uint8_t& _devType, const std::string& _version) = 0;

    virtual std::vector<std::string> getFwList(const uint8_t& _devType) = 0;

    virtual bool fwIsExisting(const uint8_t& _devType, const std::string& _version) = 0;

    virtual std::string getFwByVersion(const uint8_t& _devType, const std::string& _version) = 0;

    virtual std::string getFwByIndex(const uint8_t& _devType, uint32_t _index) = 0;
};

#endif //BSS_SDK_OTAFWMANAGERINTERFACE_H
