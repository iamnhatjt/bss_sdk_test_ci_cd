//
// Created by vnbk on 13/09/2023.
//

#include "OtaFwManager.h"

#include <utility>
#include "Logger.h"

#define TAG "OtaFwManager"

#define BP_FW_PATH          "/bp/"
#define SLAVE_FW_PATH       "/slave/"
#define PMM_FW_PATH         "/pmm/"

std::shared_ptr<OtaFwManager> OtaFwManager::create(const std::string& _path) {

}

OtaFwManager::OtaFwManager(std::string  _path) : m_rootPath(std::move(_path)){

}

bool OtaFwManager::initialized() {
    return false;
}

void OtaFwManager::loadFw(const uint8_t &_devType) {

}

bool OtaFwManager::fwIsExisting(const uint8_t &_devType, const std::string &_version) {
    return false;
}

std::string OtaFwManager::getFwByVersion(const uint8_t &_devType, const std::string &_version) {
    return "";
}

std::string OtaFwManager::getFwByIndex(const uint8_t &_devType, uint32_t _index) {
    return "";
}

std::vector<std::string> OtaFwManager::getFwList(const uint8_t &_devType) {
}

int32_t OtaFwManager::addNewFw(const uint8_t &_devType, const std::string &_version, const std::string &_path) {
    return 0;
}

int32_t OtaFwManager::removeFw(const uint8_t &_devType, const std::string &_version) {
    return 0;
}
