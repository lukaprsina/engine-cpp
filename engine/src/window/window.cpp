#include "window/window.h"

#include "platform/platform.h"
#include "vulkan_api/render_context.h"

namespace engine
{
    Window::Window(Platform &platform, WindowSettings &settings)
        : m_Platform(platform), m_Settings(settings)
    {
    }

    Window::~Window()
    {
    }

    void Window::SetSettings(WindowSettings &settings)
    {
        m_Settings = settings;
        m_Dirty = true;
    }

    void Window::DeleteRenderContext()
    {
        m_RenderContext.reset();
    }
}