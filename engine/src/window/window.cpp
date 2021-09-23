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

    Window::~Window()
    {
        if (m_Surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(m_Platform.GetApp().GetInstance().GetHandle(), m_Surface, nullptr);
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
}