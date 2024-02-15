//
// Created by vnbk on 13/09/2023.
//

#ifndef BSS_SDK_OTAFWMANAGER_H
#define BSS_SDK_OTAFWMANAGER_H

#include <string>
#include "OtaFwManagerInterface.h"

class OtaFwManager : public OtaFwManagerInterface{
public:
    static std::shared_ptr<OtaFwManager> create(const std::string& _path = "/data/selex/");
    
    void loadFw(const uint8_t& _devType) override;

    int32_t addNewFw(const uint8_t& _devType, const std::string& _version, const std::string& _path) override;

    int32_t removeFw(const uint8_t& _devType, const std::string& _version) override;

    std::vector<std::string> getFwList(const uint8_t& _devType) override;

    bool fwIsExisting(const uint8_t& _devType, const std::string& _version) override;

    std::string getFwByVersion(const uint8_t& _devType, const std::string& _version) override;

    std::string getFwByIndex(const uint8_t& _devType, uint32_t _index) override;

private:
    explicit OtaFwManager(std::string  _path);

    bool initialized();

    std::string m_rootPath;
};


#endif //BSS_SDK_OTAFWMANAGER_H
