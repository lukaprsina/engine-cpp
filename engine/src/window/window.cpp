#include "window/window.h"

#include "platform/platform.h"
#include "vulkan_api/device.h"

namespace engine
{
    Window::Window(Platform &platform, WindowSettings &settings)
        : m_Platform(platform), m_Settings(settings), m_Input(this)
    {
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