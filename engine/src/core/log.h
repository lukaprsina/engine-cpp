#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

namespace engine
{
    class Log
    {
    public:
        static void Init();

        static std::shared_ptr<spdlog::logger> &GetCoreLogger() { return s_CoreLogger; }
        static std::shared_ptr<spdlog::logger> &GetVulkanLogger() { return s_VulkanLogger; }
        static std::shared_ptr<spdlog::logger> &GetClientLogger() { return s_ClientLogger; }

    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_VulkanLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
    };
}