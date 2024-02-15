//
// Created by vnbk on 31/05/2023.
//

#ifndef BSS_BASE_TIMER_H
#define BSS_BASE_TIMER_H

#include "TimeUtils.h"

class WaitTimer{
        private:
        uint32_t m_duration;
        uint32_t m_startTime;

        public:
        explicit WaitTimer(uint32_t _duration){
            m_duration = _duration;
            m_startTime = getTickCount();
        }

        WaitTimer(const WaitTimer& _copy) = default;

        ~WaitTimer()= default;

        void reset(){
            m_startTime = getTickCount();
        }

        void reset(int32_t _duration){
            m_duration = _duration;
            m_startTime = getTickCount();
        }

        int getRemainTime() const{
            int32_t elapsed;
            elapsed = getTickCount() - m_startTime;
            if (elapsed >= m_duration)
                return 0;
            else
                return m_duration - elapsed;
        }
};

#endif //BSS_BASE_TIMER_H
