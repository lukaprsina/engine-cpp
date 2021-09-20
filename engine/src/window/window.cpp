#include "window/window.h"

#include "platform/platform.h"
#include "vulkan_api/device.h"
#include "events/event.h"
#include "events/application_event.h"

namespace engine
{
    Window::Window(Platform &platform, WindowSettings &settings)
        : m_Platform(platform), m_Settings(settings), m_Input(this)
    {
        SetEventCallback(ENG_BIND_EVENT_FN(Window::OnEvent));

        MouseMovedEvent event(this, static_cast<float>(1), static_cast<float>(2));
        m_Settings.EventCallback(event);
    }

    void Window::OnEvent(Event &event)
    {
        EventDispatcher dispatcher(event);
        m_Platform.GetApp().OnEvent(event);
    }

    void Window::SetSettings(WindowSettings &settings)
    {
        m_Settings = settings;
        m_Dirty = true;
    }

    RenderContext &Window::CreateRenderContext(Device &device,
                                               VkSurfaceKHR surface,
                                               std::vector<VkPresentModeKHR> &present_mode_priority,
                                               std::vector<VkSurfaceFormatKHR> &surface_format_priority)
    {
        m_RenderContext = std::make_unique<RenderContext>(device,
                                                          surface,
                                                          present_mode_priority,
                                                          surface_format_priority,
                                                          m_Settings.width,
                                                          m_Settings.height);

        return *m_RenderContext;
    }

    RenderContext &Window::GetRenderContext()
    {
        ENG_ASSERT(m_RenderContext, "Render context is not valid");
        return *m_RenderContext;
    }

    void Window::DeleteRenderContext()
    {
        m_RenderContext.reset();
    }
}