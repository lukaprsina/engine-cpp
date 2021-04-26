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

// Core log macros
#define ENG_CORE_TRACE(...) ::engine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define ENG_CORE_INFO(...) ::engine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define ENG_CORE_WARN(...) ::engine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define ENG_CORE_ERROR(...) ::engine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define ENG_CORE_CRITICAL(...) ::engine::Log::GetCoreLogger()->critical(__VA_ARGS__);

// Vulkan log macros
#define ENG_VULKAN_TRACE(...) ::engine::Log::GetVulkanLogger()->trace(__VA_ARGS__)
#define ENG_VULKAN_INFO(...) ::engine::Log::GetVulkanLogger()->info(__VA_ARGS__)
#define ENG_VULKAN_WARN(...) ::engine::Log::GetVulkanLogger()->warn(__VA_ARGS__)
#define ENG_VULKAN_ERROR(...) ::engine::Log::GetVulkanLogger()->error(__VA_ARGS__)
#define ENG_VULKAN_CRITICAL(...) ::engine::Log::GetVulkanLogger()->critical(__VA_ARGS__);

// Client log macros
#define ENG_TRACE(...) ::engine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define ENG_INFO(...) ::engine::Log::GetClientLogger()->info(__VA_ARGS__)
#define ENG_WARN(...) ::engine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define ENG_ERROR(...) ::engine::Log::GetClientLogger()->error(__VA_ARGS__)
#define ENG_CRITICAL(...) ::engine::Log::GetClientLogger()->critical(__VA_ARGS__);