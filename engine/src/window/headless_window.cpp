#include "window/headless_window.h"
#include "platform/platform.h"

namespace engine
{
    HeadlessWindow::HeadlessWindow(Platform &platform,
                                   WindowSettings &settings)
        : Window(platform, settings)
    {
    }

    HeadlessWindow::~HeadlessWindow()
    {
    }

    VkSurfaceKHR HeadlessWindow::CreateSurface(Instance &instance, PhysicalDevice &)
    {
        return VK_NULL_HANDLE;
    }

    bool HeadlessWindow::ShouldClose() const
    {
        return m_Closed;
    }

    void HeadlessWindow::Close()
    {
        m_Closed = true;
    }
}