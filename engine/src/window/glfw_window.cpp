#include "window/glfw_window.h"
#include "platform/platform.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace engine
{
    GlfwWindow::GlfwWindow(Platform &platform,
                           uint32_t width,
                           uint32_t height)
        : Window(platform, width, height)
    {
        if (!glfwInit())
            throw std::runtime_error("GLFW couldn't be initialized.");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        const Options &options = platform.GetApplication().GetOptions();

        //TODO: don't check everytime if it contains

        if (options.Contains("--width"))
            width = static_cast<uint32_t>(options.GetInt("--width"));

        if (options.Contains("--height"))
            width = static_cast<uint32_t>(options.GetInt("--height"));

        const char *name = platform.GetApplication().GetName().c_str();

        m_Handle = glfwCreateWindow(width, height, name, nullptr, nullptr);
    }

    GlfwWindow::~GlfwWindow()
    {
        glfwTerminate();
    }

    VkSurfaceKHR GlfwWindow::CreateSurface(Instance &instance)
    {
    }

    bool GlfwWindow::ShouldClose() const
    {
    }

    void GlfwWindow::Close()
    {
    }
}