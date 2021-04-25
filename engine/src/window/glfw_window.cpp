#include "window/glfw_window.h"
#include "platform/platform.h"

namespace engine
{
    GlfwWindow::GlfwWindow(Platform &platform,
                           uint32_t width,
                           uint32_t height)
        : Window(platform, width, height)
    {
    }

    GlfwWindow::~GlfwWindow()
    {
    }

    VkSurfaceKHR GlfwWindow::CreateSurface(Instance &instance)
    {
    }

    bool GlfwWindow::ShouldClose()
    {
    }

    void GlfwWindow::Close()
    {
    }
}