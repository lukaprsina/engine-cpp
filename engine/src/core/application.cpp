#include "core/application.h"

#include "common/vulkan_common.h"
#include "core/log.h"
#include "events/application_event.h"
#include "platform/platform.h"
#include "vulkan_api/instance/instance.h"

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

namespace engine
{
    Application::Application(Platform *platform)
        : m_Platform(platform), m_Instance(nullptr), m_ValidationLayers{}
    {
        Log::Init();

        ENG_CORE_TRACE("Logger initialized.");

        SetUsage(
            R"(Engine
    Usage:
        Engine [--headless]
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

    Application::~Application()
    {
    }

    bool Application::OnWindowClose(WindowCloseEvent & /*event*/)
    {
        m_Platform->Close();
        return true;
    }

    bool Application::Prepare()
    {
        AddInstanceExtension(m_Platform->GetSurfaceExtension());
        DebugUtilsSettings debug_utils_settings;
        m_Instance = std::make_unique<Instance>(m_Name,
                                                m_InstanceExtensions,
                                                m_ValidationLayers,
                                                debug_utils_settings,
                                                m_Headless,
                                                VK_API_VERSION_1_2);

        return true;
    }

    void Application::Finish()
    {
        ENG_CORE_TRACE("Closing Application.");
    }

    void Application::OnEvent(Event &event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
    }

    void Application::ParseOptions(std::vector<std::string> &arguments)
    {
        m_Options.ParseOptions(m_Usage, arguments);
    }
}