#include "core/log.h"
#include "spdlog/fmt/ostr.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace engine
{
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_VulkanLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

    void Log::Init()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");

        s_CoreLogger = spdlog::stdout_color_mt("Engine");
        s_CoreLogger->set_level(spdlog::level::trace);

        s_VulkanLogger = spdlog::stdout_color_mt("Vulkan");
        s_VulkanLogger->set_level(spdlog::level::trace);

        s_ClientLogger = spdlog::stdout_color_mt("App");
        s_ClientLogger->set_level(spdlog::level::trace);
    }
}