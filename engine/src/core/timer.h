#pragma once

#include <chrono>

namespace engine
{
    class Timer
    {
    public:
        Timer();
        virtual ~Timer();

        using Seconds = std::ratio<1>;
        using Milliseconds = std::ratio<1, 1000>;
        using Microseconds = std::ratio<1, 1000000>;
        using Nanoseconds = std::ratio<1, 1000000000>;

        // Configure
        using Clock = std::chrono::steady_clock;
        using DefaultResolution = Seconds;

        void Start();
        void Lap();

        template <typename T = DefaultResolution>
        double Stop()
        {
        }

        template <typename T = DefaultResolution>
        double Elapsed()
        {
        }

        template <typename T = DefaultResolution>
        double Tick()
        {
        }

        bool IsRunning() const;
    };
}
