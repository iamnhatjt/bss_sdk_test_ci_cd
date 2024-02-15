//
// Created by vnbk on 01/06/2023.
//

#ifndef BSS_PROCESS_CABINETINTERFACE_H
#define BSS_PROCESS_CABINETINTERFACE_H

#include <memory>
#include <vector>
#include <cstring>

class CabComInterface{
public:
    typedef struct CabComData{
        int32_t m_doorStt;
        int32_t m_fanStt;
        int32_t m_chargerStt;
        int32_t m_id;
        int32_t m_temp;
        int32_t m_pogoTemps[4]{};

        CabComData(){
            m_doorStt = 0;
            m_fanStt = 0;
            m_chargerStt = 0;
            m_id = 0;
            m_temp = 0;
            memset(m_pogoTemps, 0, 4);
        }

        CabComData& operator=(const CabComData& _copy){
            this->m_doorStt = _copy.m_doorStt;
            this->m_chargerStt = _copy.m_chargerStt;
            this->m_fanStt = _copy.m_fanStt;
            this->m_id = _copy.m_id;
            this->m_temp = _copy.m_temp;
            for(int index = 0; index < 4; index++){
                this->m_pogoTemps[index] = _copy.m_pogoTemps[index];
            }
            return *this;
        };
    }CabComData;

    virtual ~CabComInterface() = default;

    virtual int32_t openDoor(uint32_t _id) = 0;

    virtual int32_t getDoorStatus(uint32_t _id) = 0;

    virtual int32_t ctlFan(uint32_t _id, uint32_t _value) = 0;

    virtual int32_t getFanStatus(uint32_t _id) = 0;

    virtual int32_t ctlCharger(uint32_t _id, uint32_t _value) = 0;

    virtual int32_t getChargerStatus(uint32_t _id) = 0;

    virtual int32_t getTemp(uint32_t _id) = 0;

    virtual std::vector<int32_t> getPogoPinTemp(uint32_t _id) = 0;

    virtual int32_t getAll(uint32_t _id, CabComData& _info) = 0;
};

#endif //BSS_PROCESS_CABINETINTERFACE_H
