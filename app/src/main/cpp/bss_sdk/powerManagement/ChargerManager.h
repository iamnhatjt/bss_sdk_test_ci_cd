//
// Created by vnbk on 24/07/2023.
//

#ifndef BSS_SDK_CHARGERMANAGER_H
#define BSS_SDK_CHARGERMANAGER_H

#include <mutex>
#include <pthread.h>
#include <queue>
#include <map>

#include "ChargerManagerInterface.h"
#include "ChargerComInterface.h"
#include "BpManagerInterface.h"
#include "CabManagerInterface.h"

#define CHARGER_CMD_TIMEOUT                 (6000) // ms
#define CHARGER_CUR_MONITOR_TIME            (45*1000) // 45s
#define CHARGER_CHECK_BACK_LIST_TIMEOUT     (10*60*1000) // 10min
#define CHARGER_RETRY_NUMBER                (2)

class ChargerManager : public ChargerManagerInterface{
public:
    static std::shared_ptr<ChargerManager> create(std::shared_ptr<ChargerComInterface> _if,
                                                  std::shared_ptr<BpManagerInterface> _bpManager,
                                                  std::shared_ptr<CabManagerInterface> _cabManager,
                                                  const uint8_t& _number);

    static void bpCmdCallback(BP_COMMAND _cmd, int32_t _isSuccess, const uint32_t& _chargerId, void* _data, void* _arg);

    ~ChargerManager() override;

    int32_t setCabList(const uint8_t& _chargerId, const std::vector<int32_t>& _cabList) override;

    std::vector<int32_t> getCabList(const uint8_t& _chargerId) override;

    int32_t setChargerConfig(const uint8_t& _chargerId,  bool _isAuto, const uint32_t& _cur, const uint32_t& _vol) override;
    
     int32_t enableCab(const uint32_t& _cabId, bool _enable) override;

    int32_t charged(const uint32_t& _cabId, bool _force = false) override;

    int32_t discharged(const uint32_t& _cabId, bool _force = false) override;

    int32_t getChargerNumber() override;

    int32_t getChargerVol(const uint8_t& _chargerId) override;

    int32_t getChargerCur(const uint8_t& _chargerId) override;

    int32_t getChargerState(const uint8_t& _chargerId) override;

    int32_t getChargingCab(const uint8_t& _chargerId) override;

    void process() override;
private:
    typedef enum {
        CHARGER_CMD_NONE,
        CHARGER_CMD_CHARGING,
        CHARGER_CMD_DISCHARGING,
    }CHARGER_CMD;

    typedef enum {
        CMD_STEP_0,
        CMD_STEP_1,
        CMD_STEP_2,
    }CHARGER_CMD_STEP;

    typedef enum{
        CHARGING_CAB_ENABLE,     /// Cabinet is charged normally if BP is available
        CHARGING_CAB_DISABLE,    /// Cabinet is disable for charging.
        CHARGING_CAB_BLACKLIST   /// Cabinet is in blacklist because BP have some issues
    }CHARGING_CAB_STATE;

    typedef struct ChargerItem{
        uint8_t m_chargerId;
        CHARGER_STATE m_state;
        int32_t m_cur;
        int32_t m_vol;

        typedef struct ChargingCabItem{
            CHARGING_CAB_STATE m_chargingCabState;
            WaitTimer m_monitorTime;
            WaitTimer m_checkStateTime;

            ChargingCabItem() : m_chargingCabState(CHARGING_CAB_ENABLE),
                                m_monitorTime(CHARGER_CUR_MONITOR_TIME),
                                m_checkStateTime(3000){
            }

            void reset(){
                m_chargingCabState = CHARGING_CAB_ENABLE;
                m_monitorTime.reset();
                m_checkStateTime.reset();
            }
        }ChargingCabItem;

        std::map<int32_t, std::shared_ptr<ChargingCabItem>> m_cabIds;
        int32_t m_cabCharging;

        int32_t m_retry;
        WaitTimer m_currentMonitorTime;

        int32_t m_cmd;
        int32_t m_cmdStep;
        bool m_cmdWait;
        WaitTimer m_cmdTimeout;
        int32_t m_cmdSuccess;

        struct{
            bool m_force;
            int32_t m_cabId;
        }m_dischargeForce{};

        struct {
            bool m_force;
            int32_t m_cabId;
        }m_chargeForce{};

        explicit ChargerItem(const uint8_t& _chargerId) : m_chargerId(_chargerId),
                                m_cmdTimeout(CHARGER_CMD_TIMEOUT),
                                m_currentMonitorTime(CHARGER_CUR_MONITOR_TIME){
            m_state = CHARGER_STATE ::CHARGER_STATE_DISCHARGED;
            m_cur = 0;
            m_vol = 0;
            m_cabIds.clear();
            m_cabCharging = -1;
            m_retry = 0;

            m_cmd = CHARGER_CMD_NONE;
            m_cmdStep = CMD_STEP_0;
            m_cmdWait = false;
            m_cmdSuccess = -1;
            m_cmdTimeout.reset();

            m_chargeForce.m_force = false;
            m_chargeForce.m_cabId = -1;

            m_dischargeForce.m_cabId = -1;
            m_dischargeForce.m_force = false;
        }

        void cmdReset(){
            m_cmd = CHARGER_CMD_NONE;
            m_cmdStep = CMD_STEP_0;
            m_cmdWait = false;
            m_cmdSuccess = -1;
            m_cmdTimeout.reset();
        }

        void chargerReset(){
            cmdReset();
            m_state = CHARGER_STATE ::CHARGER_STATE_DISCHARGED;
            m_cur = 0;
            m_vol = 0;
            m_cabCharging = -1;
        }
    }ChargerItem;

    ChargerManager(std::shared_ptr<ChargerComInterface> _if,
                   std::shared_ptr<BpManagerInterface> _bpManager,
                   std::shared_ptr<CabManagerInterface> _cabManager,
                   const uint8_t& _number);

    bool initialized();

    bool initializedDefault();

    void startCmd(const std::shared_ptr<ChargerItem>& _charger, const uint32_t& _cabId, CHARGER_CMD _cmd);

    void cmdProcess(const std::shared_ptr<ChargerItem>& _charger);

    void checkBpsInCharging(const std::shared_ptr<ChargerItem>& _charger);

    int32_t ctlChargerSwitch(const uint32_t& _cabId, uint8_t _value);

    int32_t checkConditionDischarge(const std::shared_ptr<ChargerItem>& _charger);

    int32_t findCabToCharge(const std::shared_ptr<ChargerItem>& _charger);

    int32_t checkCommonChargingCondition(const uint32_t& _cabId);

    /**
     * @brief Checking temperature of cabinet. If cabinet is over threshold return TRUE, vice versa FALSE
     * @param _cab
     * @return TRUE/FALSE
     */
    bool checkTempConditionDischarging(const uint32_t& _cabId);


    bool checkChargerDischargingCondition(const std::shared_ptr<ChargerItem>& _charger);

    /**
     *
     * @param _cabId
     * @return
     */
    bool checkTempConditionCharging(const uint32_t& _cabId);

    /**
     *
     * @param _chargerId
     * @return
     */
    bool checkBpConditionCharging(const uint32_t& _cabId);

    uint8_t m_chargerNumber;
    std::vector<std::shared_ptr<ChargerItem>> m_chargers;

    std::shared_ptr<ChargerComInterface> m_if;

    std::shared_ptr<BpManagerInterface> m_bpManager;
    std::shared_ptr<CabManagerInterface> m_cabManager;

    std::mutex m_lock;
};


#endif //BSS_SDK_CHARGERMANAGER_H
