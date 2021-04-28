#include "core/application.h"

#include "core/log.h"
#include "events/application_event.h"
#include "platform/platform.h"

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

namespace engine
{
    Application::Application(Platform *platform)
        : m_Platform(platform)
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

    bool Application::OnWindowClose(WindowCloseEvent &event)
    {
        m_Platform->Close();
        return true;
    }

    bool Application::Prepare()
    {
        ENG_CORE_TRACE("{0}", m_Platform->GetSurfaceExtension());
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