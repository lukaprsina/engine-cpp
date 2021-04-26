#include "core/application.h"

#include "common/log_common.h"
#include "core/log.h"

namespace engine
{
    Application::Application()
        : m_Headless(false)
    {
        Log::Init();
        ENG_CORE_TRACE("Hello");
        ENG_VULKAN_TRACE("Hello");
        ENG_TRACE("Hello");

        SetUsage(
            R"(Engine
    Usage:
        Engine
        Engine --help
    Options:
        --help                    Show this screen.
        )"
#ifndef VK_USE_PLATFORM_DISPLAY_KHR
            R"(
        --width WIDTH             The width of the window [default: 1280].
        --height HEIGHT           The height of the window [default: 720].)"
#endif
            "\n");
    }

    void Application::ParseOptions(std::vector<std::string> &arguments)
    {
        m_Options.ParseOptions(m_Usage, arguments);
    }
}