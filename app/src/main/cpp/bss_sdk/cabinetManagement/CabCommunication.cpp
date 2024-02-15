//
// Created by vnbk on 13/07/2023.
//

#include "CabCommunication.h"
#include "Logger.h"
#include "linux_serial.h"
#include "Timer.h"

#define TAG "CabCommunication"

#define MAX_DATA_RESPONSE   32

#define CMD_TIMEOUT_DEFAULT         250
#define CMD_READ_TIMEOUT_DEFAULT    100
#define CMD_OPEN_DOOR_TIMEOUT       3000

#define DOOR_STATUS_INDEX       0
#define FAN_STATUS_INDEX        1
#define NODEID_STATUS_INDEX     2
#define CHARGER_STATUS_INDEX    3
#define CAB_TEMP_INDEX          4
#define POGO_PIN0_TEMP_INDEX    5
#define POGO_PIN1_TEMP_INDEX    6
#define POGO_PIN2_TEMP_INDEX    7
#define POGO_PIN3_TEMP_INDEX    8

std::shared_ptr<CabCommunication> CabCommunication::create(std::shared_ptr<SerialPort>& _serialPort, int32_t _baudRate) {
    auto com = std::shared_ptr<CabCommunication>(new CabCommunication(_serialPort, _baudRate));
    if(com->initialized()){
        LOG_INFO(TAG, "Created CabCommunication SUCCESS");
        return com;
    }
    return nullptr;
}

CabCommunication::CabCommunication(std::shared_ptr<SerialPort> &_serialPort, int32_t _baudRate) : m_serialPort(_serialPort),
                                                                                                  m_baudRate(_baudRate) {

}

bool CabCommunication::initialized() {
    if(!m_serialPort->isOpen()){
        if(m_serialPort->open(m_baudRate) < 0){
            LOG_ERROR(TAG, "Initialized Serial Port FAILURE");
            return false;
        }
    }

    if(m_serialPort->getBaudRate() != m_baudRate){
        if(m_serialPort->config(m_baudRate) < 0){
            LOG_ERROR(TAG, "Config Serial Port FAILURE");
            return false;
        }
    }

    return true;
}

CabCommunication::~CabCommunication(){
    if(m_serialPort->isOpen()){
        m_serialPort->close();
    }
    m_serialPort.reset();
}

int32_t CabCommunication::openDoor(uint32_t _id){
    if(!m_serialPort->isOpen() || m_baudRate != m_serialPort->getBaudRate()){
        m_serialPort->config(m_baudRate);
    }

    std::string cmd = ":";
    cmd.append(std::to_string(_id));
    cmd.append(",W,D,1*");

    /// lock
    std::lock_guard<std::mutex> lock(m_lock);

    if(m_serialPort->send((uint8_t*)cmd.c_str(), (int32_t)cmd.length(), CMD_OPEN_DOOR_TIMEOUT) < 0){
        LOG_ERROR(TAG, "Sent open Door command FAILURE");
        return -1;
    }

    uint8_t data[MAX_DATA_RESPONSE];
    int len = m_serialPort->receive(data, MAX_DATA_RESPONSE, CMD_OPEN_DOOR_TIMEOUT);

    if(len <= 0){
        LOG_ERROR(TAG, "Slave response FAILURE: len = %d", len);
    }
    return len;
}

int32_t CabCommunication::getDoorStatus(uint32_t _id){
    if(!m_serialPort->isOpen() || m_baudRate != m_serialPort->getBaudRate()){
        m_serialPort->config(m_baudRate);
    }

    std::string response = get(_id);
    if(response.empty()){
        LOG_ERROR(TAG, "Get DoorStatus from slave FAILURE");
        return -1;
    }
    std::vector<int32_t> CabComData = parse(response);

    if(!CabComData.empty()){
        return CabComData[DOOR_STATUS_INDEX];
    }

    return -1;
}

int32_t CabCommunication::ctlFan(uint32_t _id, uint32_t _value){
    if(!m_serialPort->isOpen() || m_baudRate != m_serialPort->getBaudRate()){
        m_serialPort->config(m_baudRate);
    }

    std::string cmd = ":";
    cmd.append(std::to_string(_id));
    cmd.append(",W,F,");
    cmd.append(std::to_string(_value));
    cmd.append("*");

    /// Lock
    std::lock_guard<std::mutex> lock(m_lock);

    if(m_serialPort->send((uint8_t*)cmd.c_str(), (int32_t)cmd.length(), CMD_TIMEOUT_DEFAULT) < 0){
        LOG_ERROR(TAG, "Sent ctlFan command FAILURE");
        return -1;
    }

    uint8_t data[MAX_DATA_RESPONSE];
    int len = m_serialPort->receive(data, MAX_DATA_RESPONSE, CMD_TIMEOUT_DEFAULT);

    if(len <= 0){
        LOG_ERROR(TAG, "Slave response FAILURE: len = %d", len);
    }
    return len;
}

int32_t CabCommunication::getFanStatus(uint32_t _id){
    if(!m_serialPort->isOpen() || m_baudRate != m_serialPort->getBaudRate()){
        m_serialPort->config(m_baudRate);
    }

    std::string response = get(_id);
    if(response.empty()){
        LOG_ERROR(TAG, "Get Fan status from slave FAILURE");
        return -1;
    }
    std::vector<int32_t> CabComData = parse(response);

    if(!CabComData.empty()){
        return CabComData[FAN_STATUS_INDEX];
    }

    return -1;
}

int32_t CabCommunication::ctlCharger(uint32_t _id, uint32_t _value) {
    if(!m_serialPort->isOpen() || m_baudRate != m_serialPort->getBaudRate()){
        m_serialPort->config(m_baudRate);
    }

    std::string cmd = ":";
    cmd.append(std::to_string(_id));
    cmd.append(",W,C,");
    cmd.append(std::to_string(_value));
    cmd.append("*");

    /// lock
    std::lock_guard<std::mutex> lock(m_lock);

    if(m_serialPort->send((uint8_t*)cmd.c_str(), (int32_t)cmd.length(), CMD_TIMEOUT_DEFAULT) < 0){
        LOG_ERROR(TAG, "Sent ctlCharger command FAILURE");
        return -1;
    }

    uint8_t data[MAX_DATA_RESPONSE];
    int len = m_serialPort->receive(data, MAX_DATA_RESPONSE, CMD_TIMEOUT_DEFAULT);

    if(len <= 0){
        LOG_ERROR(TAG, "Slave response FAILURE: len = %d", len);
    }
    return len;
}

int32_t CabCommunication::getChargerStatus(uint32_t _id) {
    if(!m_serialPort->isOpen() || m_baudRate != m_serialPort->getBaudRate()){
        m_serialPort->config(m_baudRate);
    }

    std::string response = get(_id);
    if(response.empty()){
        LOG_ERROR(TAG, "Get ChargerCom from slave FAILURE");
        return -1;
    }
    std::vector<int32_t> CabComData = parse(response);

    if(!CabComData.empty()){
        return CabComData[CHARGER_STATUS_INDEX];
    }

    return -1;
}

int32_t CabCommunication::getTemp(uint32_t _id){
    if(!m_serialPort->isOpen() || m_baudRate != m_serialPort->getBaudRate()){
        m_serialPort->config(m_baudRate);
    }

    std::string response = get(_id);
    if(response.empty()){
        LOG_ERROR(TAG, "Get Cabinet temp from slave FAILURE");
        return -1;
    }
    std::vector<int32_t> CabComData = parse(response);

    if(!CabComData.empty()){
        return CabComData[CAB_TEMP_INDEX];
    }

    return -1;
}

std::vector<int32_t> CabCommunication::getPogoPinTemp(uint32_t _id){
    std::vector<int32_t> pogoTemps;
    std::string response = get(_id);
    if(response.empty()){
        LOG_ERROR(TAG, "Get PogoPinTemp from slave FAILURE");
        return pogoTemps;
    }
    std::vector<int32_t> CabComData = parse(response);

    if(!CabComData.empty()){
        pogoTemps.push_back(CabComData[POGO_PIN0_TEMP_INDEX]);
        pogoTemps.push_back(CabComData[POGO_PIN1_TEMP_INDEX]);
        pogoTemps.push_back(CabComData[POGO_PIN2_TEMP_INDEX]);
        pogoTemps.push_back(CabComData[POGO_PIN3_TEMP_INDEX]);
    }

    return pogoTemps;
}

int32_t CabCommunication::setNodeId(const uint32_t& _id, NodeIdControllerInterface::CANBUS_NODE_ID _up) {
    if(!m_serialPort->isOpen() || m_baudRate != m_serialPort->getBaudRate()){
        m_serialPort->config(m_baudRate);
    }

    std::string cmd = ":";
    cmd.append(std::to_string(_id));
    cmd.append(",W,N,");
    cmd.append(std::to_string((int32_t)_up));
    cmd.append("*");

    /// Lock thread
    std::lock_guard<std::mutex> lock(m_lock);

    if(m_serialPort->send((uint8_t*)cmd.c_str(), (int32_t)cmd.length(), CMD_TIMEOUT_DEFAULT) < 0){
        LOG_ERROR(TAG, "Sent setNodeId command FAILURE");
        return -1;
    }

//    LOG_DEBUG(TAG, "Set Node ID slave %d to %s", _id, (int32_t)_up ? "HIGH" : "LOW" );

    uint8_t data[MAX_DATA_RESPONSE] = {0,};
    int len = m_serialPort->receive(data, MAX_DATA_RESPONSE, CMD_TIMEOUT_DEFAULT);

    return len;
}

int32_t CabCommunication::getNodeId(const uint32_t& _id) {
    std::string response = get(_id);
    if(response.empty()){
        LOG_ERROR(TAG, "Get NodeID from slave FAILURE");
        return -1;
    }
    std::vector<int32_t> CabComData = parse(response);

    if(!CabComData.empty()){
        return CabComData[NODEID_STATUS_INDEX];
    }

    return -1;
}

std::string CabCommunication::get(uint32_t _id) {
    if(!m_serialPort->isOpen() || m_baudRate != m_serialPort->getBaudRate()){
        m_serialPort->config(m_baudRate);
    }

    std::string cmd = ":";
    cmd.append(std::to_string(_id));
    cmd.append(",R,A*");

    /// Lock thread
    std::lock_guard<std::mutex> lock(m_lock);

    if(m_serialPort->send((uint8_t*)cmd.c_str(), (int32_t)cmd.length(), CMD_TIMEOUT_DEFAULT) < 0){
        LOG_ERROR(TAG, "Sent get command FAILURE");
        return "";
    }

    uint8_t data[MAX_DATA_RESPONSE] = {0, };
    int len = m_serialPort->receive(data, MAX_DATA_RESPONSE, CMD_READ_TIMEOUT_DEFAULT);

    if(len > 0){
        std::string response((const char*)data, len);
//        LOG_DEBUG(TAG, "Get All response from slave %d: %s",_id, response.c_str());
        return response;
    }else{
//        LOG_ERROR(TAG, "Get All Slave %d response FAILURE: len = %d", _id, len);
        return "";
    }
}

std::vector<int32_t> CabCommunication::parse(const std::string &_message) {
    WaitTimer timeout(300);
    std::vector<int32_t> cabComData;
    std::string message = _message;
    std::string::size_type pos = 0;

    if(message.find(':') == std::string::npos){
        LOG_ERROR(TAG, "Message INVALID, missing ':' starting character");
        return cabComData;
    }
    if(message.find('*') == std::string::npos){
        LOG_ERROR(TAG, "Message INVALID, missing '*' stopping character");
        return cabComData;
    }

    pos = message.find('R');
    if(pos == std::string::npos){
        return cabComData;
    }
    message = message.substr(pos+2);

    while (pos != std::string::npos && timeout.getRemainTime()){
        int value = 0;
        try {
            value = std::stoi(message);
        }
        catch (...){
            continue;
        }
        cabComData.push_back(value);
        pos = message.find(',');
        message = message.substr(pos+1);

        if(cabComData.size() > POGO_PIN3_TEMP_INDEX){
            break;
        }
    }

    if(!timeout.getRemainTime()){
        LOG_ERROR(TAG, "Timeout parsing message from cabinet");
        cabComData.clear();
    }

    return cabComData;
}

int32_t CabCommunication::getAll(uint32_t _id, CabComInterface::CabComData& _info) {
    std::string response = get(_id);
    if(response.empty()){
//        LOG_ERROR(TAG, "Get all data from slave FAILURE");
        return -1;
    }
    std::vector<int32_t> cabComData = parse(response);

    if(!cabComData.empty()){
        _info.m_fanStt = cabComData[FAN_STATUS_INDEX];
        _info.m_doorStt = !cabComData[DOOR_STATUS_INDEX];
        _info.m_chargerStt = !cabComData[CHARGER_STATUS_INDEX];
        _info.m_id = !cabComData[NODEID_STATUS_INDEX];
        _info.m_temp = cabComData[CAB_TEMP_INDEX];

        if(cabComData.size() > POGO_PIN0_TEMP_INDEX) {
            _info.m_pogoTemps[0] = cabComData[POGO_PIN0_TEMP_INDEX];
            _info.m_pogoTemps[1] = cabComData[POGO_PIN1_TEMP_INDEX];
            _info.m_pogoTemps[2] = cabComData[POGO_PIN2_TEMP_INDEX];
            _info.m_pogoTemps[3] = cabComData[POGO_PIN3_TEMP_INDEX];
        }else{
            _info.m_pogoTemps[0] = -1;
            _info.m_pogoTemps[1] = -1;
            _info.m_pogoTemps[2] = -1;
            _info.m_pogoTemps[3] = -1;
        }

        return (int32_t)cabComData.size();
    }

    return -1;
}