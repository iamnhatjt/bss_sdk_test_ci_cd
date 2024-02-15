//
// Created by vnbk on 21/09/2023.
//
#include <thread>
#include <mutex>
#include <condition_variable>

#include "Logger.h"
#include "SerialPort.h"

#include "BpCoCom.h"
#include "BpManagement.h"

#define MAIN_TAG "BP_ASSIGNING_TEST"

#define BP_NUMBER   5

typedef struct BpAssigningStatistic{
    int32_t m_successNum{0};
    int32_t m_failureNum{0};
    int32_t m_total{0};

    std::atomic_bool m_waitAssigning{false};
}BpAssigningStatistic;

class BpObserverTest : public BpObserverInterface{
public:
    BpObserverTest(){
        for(int i = 0; i < BP_NUMBER; i++){
            auto item = std::make_shared<BpAssigningStatistic>();
            m_bpStatistics.push_back(item);
        }
    }

    void onBpConnected(const uint32_t& _id, const std::string& _sn, const std::string& _version) override{
        LOG_INFO(MAIN_TAG, "Bp is Connected: node id: %d, SN: %s", _id, _sn.c_str());
    }

    void onBpDisconnected(const uint32_t& _id, const std::string& _sn) override{
        LOG_WARN(MAIN_TAG, "Bp is disconnected: node id: %d, SN: %s", _id, _sn.c_str());
    }

    void onBpAssignStatus(const uint32_t& _id, int32_t _status) override{
//        LOG_DEBUG(MAIN_TAG, "Assigning Bp status : node id: %d, status: %d", _id, _status);
        if(_status == (int32_t)BP_ASSIGN_STATE::BP_ASSIGNED){
            m_bpStatistics[_id]->m_successNum++;
            m_bpStatistics[_id]->m_total++;

            m_bpStatistics[_id]->m_waitAssigning = false;
        }else if(_status == (int32_t)BP_ASSIGN_STATE::BP_ASSIGN_FAILURE){
            m_bpStatistics[_id]->m_failureNum++;
            m_bpStatistics[_id]->m_total++;

            LOG_ERROR(MAIN_TAG, "Assigning BP %d is FAILURE at %d time, Number of FAILURE :%d, Number of SUCCESS :%d",
                      _id,
                      m_bpStatistics[_id]->m_total,
                      m_bpStatistics[_id]->m_failureNum,
                      m_bpStatistics[_id]->m_successNum);

            m_bpStatistics[_id]->m_waitAssigning = false;
        }
    }

    void onDeviceIsAssigned(const uint32_t& _id, int32_t _isSuccess, const std::string& _bp_sn, const std::string& _dev_type, const std::string& _dev_sn) override{
//        LOG_DEBUG(MAIN_TAG, "Bp is Connected: node id: %d, SN: %s, Version: %s", _id, _sn.c_str(), _dev_sn.c_str());
    }

    std::vector<std::shared_ptr<BpAssigningStatistic>> m_bpStatistics;
};

class NodeIdController : public NodeIdControllerInterface{
public:

    explicit NodeIdController(std::shared_ptr<SerialPort>& _serialPort) : m_serialPort(_serialPort){

    }

    int32_t setNodeId(const uint32_t& _node_id, CANBUS_NODE_ID _up) override{
        if(!m_serialPort->isOpen()){
            if(m_serialPort->open(115200) < 0){
                LOG_ERROR(MAIN_TAG, "Set NodeId FAILURE, Reason: Could NOT open Serial Port");
                return -1;
            }
        }
//        LOG_DEBUG(MAIN_TAG, "Node ID %d to %s",_node_id, (int32_t)_up ? "DESELECT" : "SELECT");

        std::string nodeIdCtl = ":";
        nodeIdCtl.append(std::to_string(_node_id));
        nodeIdCtl.append(",W,N,");
        nodeIdCtl.append(std::to_string((int)_up));
        nodeIdCtl.append("*");

//        LOG_INFO(MAIN_TAG, "Command control NODE ID %d: %s", _node_id, nodeIdCtl.c_str());

        int32_t len = m_serialPort->send((const uint8_t*)nodeIdCtl.c_str(), nodeIdCtl.length(), 500);
        if(len < 0){
            LOG_ERROR(MAIN_TAG, "Set NodeID FAILURE, Reason: Could NOT send control NODEID command");
            return -1;
        }

        uint8_t data[32] = {0,};
        len = m_serialPort->receive(data, 32, 200);
        if(len > 0){
            /*std::string response((const char*)data, len);
            LOG_DEBUG(MAIN_TAG, "Response from slave: %s", response.c_str());*/
        }else{
//            LOG_ERROR(MAIN_TAG, "Slave response FAILURE: len = %d", len);
            return -1;
        }
        return len;
    }

    int32_t getNodeId(const uint32_t& _node_id) override{
        LOG_DEBUG(MAIN_TAG, "Get Node ID");
        return 0;
    }

private:
    std::shared_ptr<SerialPort> m_serialPort;
};

static std::shared_ptr<BpManager> g_bpManager = nullptr;

std::thread g_bpManagerThread;
std::atomic_bool g_bpShutdown{false};
void bp_management_thread() {
    while (!g_bpShutdown){
        g_bpManager->process();
    }
}

void log_show(const char* _log){
    printf("%s\n", _log);
}

#define ASSIGNING_TESTING_TOTAL     1000
#define TIME_BETWEEN_TWO_ASSIGNING  2000 //ms = 2s

static int32_t g_count = 0;
static int32_t g_currentBp = 0;

static int32_t g_bp_number = BP_NUMBER;
static int32_t g_testing_total = ASSIGNING_TESTING_TOTAL;
static int32_t g_time_between_two_assigning = TIME_BETWEEN_TWO_ASSIGNING;

//#define CO_ETHERNET_INTERFACE
#define CO_SERIAL_INTERFACE

int main(int argc, const char *argv[]) {
    auto logger = new Logger(Logger::LOG_LEVEL_DEBUG, nullptr, true);
    logger->setLogPutFn(log_show);

    LOG_INFO(MAIN_TAG, "/******************** Help *******************/");
    LOG_INFO(MAIN_TAG, "Configure assigning bp: ");
    LOG_INFO(MAIN_TAG, "./bp_assigning bp_number total_testing time_between_assigning\n");

    if(argc == 2){
        g_bp_number = strtol(argv[1], nullptr, 10);
    }else if(argc == 3){
        g_bp_number = strtol(argv[1], nullptr, 10);
        g_testing_total = strtol(argv[2], nullptr, 10);
    }else if(argc == 4) {
        g_bp_number = strtol(argv[1], nullptr, 10);
        g_testing_total = strtol(argv[2], nullptr, 10);
        g_time_between_two_assigning = strtol(argv[3], nullptr, 10);
    }else{
        g_bp_number = BP_NUMBER;
        g_testing_total = ASSIGNING_TESTING_TOTAL;
        g_time_between_two_assigning = TIME_BETWEEN_TWO_ASSIGNING;
    }

    LOG_INFO(MAIN_TAG, "\n/******************** ASSIGNING BP TESTING *******************/");
    LOG_INFO(MAIN_TAG, "Assigning BP testing Info: ");
    LOG_INFO(MAIN_TAG, " - DateTime for testing: %s", DateTime::getCurrentDateTime()->convertDatetimeToString(DD_MM_YYYY_hh_mm_ss).c_str());
    LOG_INFO(MAIN_TAG, " - Total BP for testing: %d", g_bp_number);
    LOG_INFO(MAIN_TAG, " - Total Number for testing: %d", g_testing_total);
    LOG_INFO(MAIN_TAG, " - Time between two assigning time: %d ms", g_time_between_two_assigning);
    LOG_INFO(MAIN_TAG, "\n/**************************************************************/\n");

    LOG_INFO(MAIN_TAG, "\n/************ Initializing BP Management module for testing **************/");
    LOG_INFO(MAIN_TAG, "Creating Cabinet RS485 for NodeId Controller");
    auto cabSerial = std::make_shared<SerialPort>("/dev/ttyUSB0");
    if(cabSerial->open(CAB_SERIAL_PORT_BAUD) < 0){
        LOG_ERROR(MAIN_TAG, "Could NOT open Serial PORT");
        return EXIT_FAILURE;
    }

    LOG_INFO(MAIN_TAG, "Creating NodeID Controller");
    auto nodeIdController = std::make_shared<NodeIdController>(cabSerial);


#ifdef CO_SERIAL_INTERFACE
    LOG_INFO(MAIN_TAG, "Creating CanOpen Interface via Serial Port");
    sm_co_if_t* coInterface = sm_co_if_create_default(CO_SERIAL_CANBUS_IF,
                                                      "/dev/ttyS0",
                                                      115200,
                                                      nullptr);
#endif

#ifdef CO_ETHERNET_INTERFACE
    LOG_INFO(MAIN_TAG, "Creating CanOpen Interface via Serial Port");
    sm_co_if_t* coInterface = sm_co_if_create_default(CO_ETHERNET_CANBUS_IF,
                                                      HOST_DEFAULT,
                                                      PORT_DEFAULT,
                                                      nullptr);
#endif

    LOG_INFO(MAIN_TAG, "Creating BP Communication Interface. Phase 1: CanOpen, Phase 2: Modbus. Set assigning automatically");
    auto bpComInterface = BpCoCom::create(nodeIdController,
                                          coInterface,
                                          g_bp_number);

    LOG_INFO(MAIN_TAG, "Creating BP Management Service");
    g_bpManager = BpManager::create(bpComInterface, g_bp_number);
    if(!g_bpManager){
        LOG_ERROR(MAIN_TAG, "Created BP Management service FAILURE");
        return -1;
    }
    bpComInterface->addManager(g_bpManager);

    auto bpObserver = std::make_shared<BpObserverTest>();
    g_bpManager->addObserver(bpObserver);

    g_bpManagerThread = std::thread(&bp_management_thread);
    LOG_INFO(MAIN_TAG, "\n/******************** Finished initializing progress *******************/\n");

    LOG_INFO(MAIN_TAG, "\n/******************** Start TESTING *******************/");
    while (true) {
        if (!bpObserver->m_bpStatistics[g_currentBp]->m_waitAssigning) {
            g_bpManager->assign(g_currentBp);
            bpObserver->m_bpStatistics[g_currentBp]->m_waitAssigning = true;
        }

        while (bpObserver->m_bpStatistics[g_currentBp]->m_waitAssigning);

        g_currentBp++;
        if (g_currentBp >= g_bp_number) {
            g_currentBp = 0;
            g_count++;

            if (g_count >= g_testing_total) {
                break;
            }

            LOG_WARN(MAIN_TAG, "Finished the assigning BP process %d time", g_count);
            LOG_WARN(MAIN_TAG, "Reset All BPs and Try again");
            for(int i = 0; i < g_bp_number; i++){
                nodeIdController->setNodeId(i, NodeIdControllerInterface::CANBUS_NODE_ID::NODE_ID_LOW);
                g_bpManager->resetBp(i);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(g_time_between_two_assigning));
        }
    }
    g_bpShutdown = true;

    if(g_bpManagerThread.joinable()){
        g_bpManagerThread.join();
    }

    LOG_INFO(MAIN_TAG, "\n/******************* Finished ASSIGNING BP TESTING *******************/");
    LOG_INFO(MAIN_TAG, "This is the result of the TESTING");
    for(auto& item : bpObserver->m_bpStatistics){
        LOG_INFO(MAIN_TAG, "Bp %d have SUCCESS: %d, FAILURE: %d, TOTAL: %d",
                 item.get(),
                 item->m_successNum,
                 item->m_failureNum,
                 item->m_total);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_BETWEEN_TWO_ASSIGNING));

    LOG_INFO(MAIN_TAG, "\n/******************* END *******************/");
    return EXIT_SUCCESS;
}
