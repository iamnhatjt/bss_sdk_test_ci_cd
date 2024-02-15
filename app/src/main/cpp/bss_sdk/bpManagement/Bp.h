//
// Created by vnbk on 07/07/2023.
//

#ifndef BSS_SDK_BP_H
#define BSS_SDK_BP_H

#include <cstring>
#include <algorithm>

#include "bss_data_def.h"
#include "BpManagerInterface.h"

#include "Timer.h"

#define DEVICE_VERSION_LENGTH       32
#define DEVICE_SN_LENGTH            DEVICE_VERSION_LENGTH
#define BP_CONNECTION_TIMEOUT       10000

#define BP_ASSIGNING_RETRIED_MAX    5

#define BP_CMD_TIMEOUT             5000 /// ms
enum {
    BP_CMD_FAILURE,
    BP_CMD_SUCCESS,
};

enum class BP_COMMAND{
    BP_CMD_REBOOT,
    BP_CMD_CHARGED,
    BP_CMD_DISCHARGED,
    BP_CMD_READ_SN,
    BP_CMD_READ_ASSIGNED_DEVICE,
    BP_CMD_WRITE_ASSIGNED_DEVICE,
    BP_CMD_READ_VERSION,
    BP_CMD_NUMBER
};

typedef void (*BpCmdCallback_t)(BP_COMMAND, int32_t, const uint32_t&, void*, void*);

static inline std::string convertBpCmdToString(BP_COMMAND _cmd){
    switch (_cmd) {
        case BP_COMMAND::BP_CMD_REBOOT:
            return "BP_CMD_REBOOT";
        case BP_COMMAND::BP_CMD_CHARGED:
            return "BP_CMD_CHARGED";
        case BP_COMMAND::BP_CMD_DISCHARGED:
            return "BP_CMD_DISCHARGED";
        case BP_COMMAND::BP_CMD_READ_SN:
            return "BP_CMD_READ_SN";
        case BP_COMMAND::BP_CMD_READ_ASSIGNED_DEVICE:
            return "BP_CMD_READ_ASSIGNED_DEVICE";
        case BP_COMMAND::BP_CMD_WRITE_ASSIGNED_DEVICE:
            return "BP_CMD_WRITE_ASSIGNED_DEVICE";
        case BP_COMMAND::BP_CMD_READ_VERSION:
            return "BP_CMD_READ_VERSION";
        default:
            return "BP_CMD_UNKNOWN";
    }
}

enum class BP_ASSIGN_STATE{
    BP_ASSIGN_WAIT_REQUEST,
    BP_ASSIGN_START,
    BP_ASSIGN_SLAVE_SELECT,
    BP_ASSIGN_SLAVE_SELECT_CONFIRM,
    BP_ASSIGN_WAIT_CONFIRM,
    BP_ASSIGN_AUTHORIZING_START,
    BP_ASSIGN_AUTHORIZING,
    BP_ASSIGNED,
    BP_ASSIGN_FAILURE
};


static inline std::string convertBpAssignStateToString(BP_ASSIGN_STATE _state){
    switch (_state) {
        case BP_ASSIGN_STATE::BP_ASSIGN_WAIT_REQUEST:
            return "BP_ASSIGN_WAIT_REQUEST";
        case BP_ASSIGN_STATE::BP_ASSIGN_START:
            return "BP_ASSIGN_START";
        case BP_ASSIGN_STATE::BP_ASSIGN_SLAVE_SELECT:
            return "BP_ASSIGN_SLAVE_SELECT";
        case BP_ASSIGN_STATE::BP_ASSIGN_SLAVE_SELECT_CONFIRM:
            return "BP_ASSIGN_SLAVE_SELECT_CONFIRM";
        case BP_ASSIGN_STATE::BP_ASSIGN_WAIT_CONFIRM:
            return "BP_ASSIGN_WAIT_CONFIRM";
        case BP_ASSIGN_STATE::BP_ASSIGN_AUTHORIZING_START:
            return "BP_ASSIGN_AUTHORIZING_START";
        case BP_ASSIGN_STATE::BP_ASSIGN_AUTHORIZING:
            return "BP_ASSIGN_AUTHORIZING";
        case BP_ASSIGN_STATE::BP_ASSIGNED:
            return "BP_ASSIGNED";
        case BP_ASSIGN_STATE::BP_ASSIGN_FAILURE:
            return "BP_ASSIGN_FAILURE";
        default:
            return "BP_ASSIGN_UNKNOWN";
    }
}

static inline bool verifyAsciiString(const std::string& _value, std::string& _newValue){
    for(auto& item : _value){
        if(item >= '0' && item <= '9') {
            _newValue += item;
            continue;
        }
        if(item >= 'A' && item <= 'Z'){
            _newValue += item;
            continue;
        }
        if(item >= 'a' && item <= 'z'){
            _newValue += item;
            continue;
        }
        if(item == '.'){
            _newValue += item;
            continue;
        }
        if(item == '_'){
            _newValue += item;
            continue;
        }
        return false;
    }
    return true;
}

enum BP_INITIALIZED_STEP{
    BP_INITIALIZED_STEP_0,
    BP_INITIALIZED_STEP_1,

    BP_INITIALIZED_STEP_NUMBER
};

class Bp {
public:
    typedef struct BpCmd{
        std::shared_ptr<Bp> m_bp;
        BP_COMMAND m_cmd;
        void* m_data;
        BpCmdCallback_t m_callback;
        void* m_arg;
        WaitTimer m_timeout;

        BpCmd(const std::shared_ptr<Bp>& _bp, BP_COMMAND _cmd, void* _data, BpCmdCallback_t _fn, void* _arg) : m_timeout(BP_CMD_TIMEOUT){
            m_bp = _bp;
            m_cmd = _cmd;
            m_data = _data;
            m_callback = _fn;
            m_arg = _arg;
        }
    }BpCmd;

    ~Bp(){
        reset();
    }
    uint32_t m_id;
    BpData m_info;

    BP_ASSIGN_STATE m_assignStatus;
    WaitTimer m_timeout;

    uint32_t m_assigningRetried;

    uint8_t m_sn[DEVICE_SN_LENGTH]{};
    uint8_t m_version[DEVICE_VERSION_LENGTH]{};
    uint8_t m_assignedSn[DEVICE_SN_LENGTH]{};
    uint8_t m_chargingState;
    uint8_t m_mattingState;
    uint8_t m_rebootState;

    bool m_waitingCmd;
    uint8_t m_initStep;

    explicit Bp(const uint32_t& _node_id) : m_timeout(BP_CONNECTION_TIMEOUT){
        m_id = _node_id;
        reset();
    }

    Bp(const Bp& _item) : m_timeout(BP_CONNECTION_TIMEOUT){
        this->m_assignStatus = _item.m_assignStatus;
        this->m_id = _item.m_id;
        this->m_info = _item.m_info;
        memcpy(m_sn, _item.m_sn, DEVICE_SN_LENGTH);
        memcpy(m_version, _item.m_version, DEVICE_VERSION_LENGTH);
        memcpy(m_assignedSn, _item.m_assignedSn, DEVICE_SN_LENGTH);
        this->m_chargingState = _item.m_chargingState;
        this->m_mattingState = _item.m_mattingState;
        this->m_rebootState = _item.m_rebootState;
        this->m_assigningRetried = _item.m_assigningRetried;
        this->m_waitingCmd = false;
        this->m_initStep = 0;
    }

    void reset(){
        this->m_assignStatus = BP_ASSIGN_STATE::BP_ASSIGN_FAILURE;
        this->m_info.m_sn.clear();
        this->m_info.m_version.clear();
        this->m_info.m_assignedSn.clear();
        this->m_info.m_soc = 0;
        this->m_info.m_soh = 0;
        this->m_info.m_cycle = 0;
        this->m_info.m_vol = 0;
        this->m_info.m_cur = 0;
        memset(this->m_info.m_temps, 0, BP_CELL_TEMP_SIZE);
        memset(this->m_info.m_cellVols, 0, BP_CELL_VOL_SIZE);
        this->m_timeout.reset();
        memset(this->m_sn, '\0', DEVICE_SN_LENGTH);
        memset(this->m_version, '\0', DEVICE_VERSION_LENGTH);
        memset(this->m_assignedSn, '\0', DEVICE_SN_LENGTH);
        this->m_chargingState = 0;
        this->m_mattingState = 0;
        this->m_rebootState = 0;
        this->m_assigningRetried = 0;
        this->m_waitingCmd = false;
        this->m_initStep = 0;
    }

    Bp& operator=(const Bp& _other){
        this->m_id = _other.m_id;
        this->m_info = _other.m_info;
        this->m_assignStatus = _other.m_assignStatus;
        this->m_timeout = _other.m_timeout;
        memcpy(this->m_sn, _other.m_sn, DEVICE_SN_LENGTH);
        memcpy(this->m_version, _other.m_version, DEVICE_VERSION_LENGTH);
        memcpy(this->m_assignedSn, _other.m_assignedSn, DEVICE_SN_LENGTH);
        this->m_chargingState = _other.m_chargingState;
        this->m_mattingState = _other.m_mattingState;
        this->m_rebootState = _other.m_rebootState;
        this->m_assigningRetried = _other.m_assigningRetried;
        this->m_waitingCmd = _other.m_waitingCmd;
        this->m_initStep = _other.m_initStep;

        return *this;
    }
};


#endif //BSS_SDK_BP_H
