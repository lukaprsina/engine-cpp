#include "core/timer.h"

namespace engine
{
    Timer::Timer()
        : m_Running(false), m_Lapping(false),
          m_StartTime(Clock::now()), m_LapTime(Clock::now()),
          m_PreviousTick(Clock::now())
    {
    }

    void Timer::Start()
    {
        if (!m_Running)
        {
            m_Running = true;
            m_StartTime = Clock::now();
        }
    }

    void Timer::Lap()
    {
        m_Lapping = true;
        m_LapTime = Clock::now();
    }
}
