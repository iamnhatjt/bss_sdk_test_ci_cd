//
// Created by vnbk on 13/07/2023.
//

#ifndef BSS_SDK_CABCOMMUNICATION_H
#define BSS_SDK_CABCOMMUNICATION_H

#include <string>
#include <pthread.h>
#include <mutex>
#include <atomic>

#include "CabComInterface.h"
#include "NodeIdControllerInterface.h"
#include "SerialPort.h"

class CabCommunication : public CabComInterface,
                         public NodeIdControllerInterface{
public:
    static std::shared_ptr<CabCommunication> create(std::shared_ptr<SerialPort>& _serialPort, int32_t _baudRate = 115200);

    ~CabCommunication() override;

    int32_t openDoor(uint32_t _id)override;

    int32_t getDoorStatus(uint32_t _id) override;

    int32_t ctlFan(uint32_t _id, uint32_t _value)override;

    int32_t getFanStatus(uint32_t _id)override;

    int32_t ctlCharger(uint32_t _id, uint32_t _value) override;

    int32_t getChargerStatus(uint32_t _id) override;

    int32_t getTemp(uint32_t _id)override;

    std::vector<int32_t> getPogoPinTemp(uint32_t _id)override;

    int32_t setNodeId(const uint32_t& _id, CANBUS_NODE_ID _up) override;

    int32_t getNodeId(const uint32_t& _id) override;

    int32_t getAll(uint32_t _id, CabComData& _info) override;

private:
    CabCommunication(std::shared_ptr<SerialPort>& _serialPort, int32_t _baudRate);

    bool initialized();

    std::string get(uint32_t _id);

    std::vector<int32_t> parse(const std::string& _message);

    std::shared_ptr<SerialPort> m_serialPort;
    int32_t m_baudRate;

    std::mutex m_lock;
};


#endif //BSS_SDK_CABCOMMUNICATION_H
