#include "window/headless_window.h"
#include "platform/platform.h"

namespace engine
{
    HeadlessWindow::HeadlessWindow(Platform &platform,
                                   uint32_t width,
                                   uint32_t height)
        : Window(platform, width, height)
    {
    }

    HeadlessWindow::~HeadlessWindow()
    {
    }

    VkSurfaceKHR HeadlessWindow::CreateSurface(Instance &instance)
    {
    }

    bool HeadlessWindow::ShouldClose()
    {
    }

    void HeadlessWindow::Close()
    {
    }
}