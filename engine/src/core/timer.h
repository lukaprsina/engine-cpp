#pragma once

#include <chrono>

namespace engine
{
    class Timer
    {
    public:
        Timer();
        virtual ~Timer() = default;

        using Seconds = std::ratio<1>;
        using Milliseconds = std::ratio<1, 1000>;
        using Microseconds = std::ratio<1, 1000000>;
        using Nanoseconds = std::ratio<1, 1000000000>;

        // Configure
        using Clock = std::chrono::steady_clock;
        using DefaultUnit = Seconds;

        void Start();
        void Lap();

        template <typename T = DefaultUnit>
        double Stop()
        {
            if (!m_Running)
            {
                return 0;
            }

            m_Running = false;
            m_Lapping = false;
            auto duration = std::chrono::duration<double, T>(Clock::now() - m_StartTime);
            m_StartTime = Clock::now();
            m_LapTime = Clock::now();

            return duration.count();
        }

        template <typename T = DefaultUnit>
        double Elapsed()
        {
            if (!m_Running)
            {
                return 0;
            }

            Clock::time_point start = m_StartTime;

            if (m_Lapping)
            {
                start = m_LapTime;
            }

            return std::chrono::duration<double, T>(Clock::now() - start).count();
        }

        template <typename T = DefaultUnit>
        double Tick()
        {
            auto now = Clock::now();
            auto duration = std::chrono::duration<double, T>(now - m_PreviousTick);
            m_PreviousTick = now;
            return duration.count();
        }

        bool IsRunning() const { return m_Running; }

    private:
        bool m_Running;
        bool m_Lapping;

        Clock::time_point m_StartTime;
        Clock::time_point m_LapTime;
        Clock::time_point m_PreviousTick;
    };
}
