//
// Created by vnbk on 24/07/2023.
//

#ifndef BSS_SDK_EMETERMANAGERINTERFACE_H
#define BSS_SDK_EMETERMANAGERINTERFACE_H

#include <string>

class EMeterManagerInterface{
public:
    typedef struct EMeterInfo{
        float m_vol;
        float m_cur;
        float m_cos;
        float m_freq;
        float m_kwh;

        EMeterInfo(){
            m_vol = 0;
            m_cos = 0;
            m_cur = 0;
            m_freq = 0;
            m_kwh = 0;
        };

        EMeterInfo& operator=(const EMeterInfo& _other) = default;
    }EMeterInfo;

    virtual ~EMeterManagerInterface() = default;

    virtual const std::string getManufactureActive() = 0;

    virtual void setReadingPeriod(int32_t _period) = 0;

    virtual EMeterInfo& getAll() = 0;

    virtual float getVol() = 0;

    virtual float getCur() = 0;

    virtual float getCos() = 0;

    virtual float getFreq() = 0;

    virtual float getKwh() = 0;

    virtual void process() = 0;
};

#endif //BSS_SDK_EMETERMANAGERINTERFACE_H
