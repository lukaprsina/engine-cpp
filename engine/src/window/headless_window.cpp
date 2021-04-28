#include "window/headless_window.h"
#include "platform/platform.h"

namespace engine
{
    HeadlessWindow::HeadlessWindow(Platform &platform,
                                   uint32_t width,
                                   uint32_t height)
        : Window(platform, width, height), m_Closed(false)
    {
    }

    HeadlessWindow::~HeadlessWindow()
    {
    }

    VkSurfaceKHR HeadlessWindow::CreateSurface(Instance &instance)
    {
        return nullptr;
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