#include "window/glfw_window.h"
#include "platform/platform.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace engine
{
    namespace
    {
        void ErrorCallback(int error, const char *description)
        {
            ENG_CORE_TRACE("GLFW Error (code {}): {}", error, description);
        }

        void WindowCloseCallback(GLFWwindow *window)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        void WindowSizeCallback(GLFWwindow *window, int width, int height)
        {
        }
    }

    GlfwWindow::GlfwWindow(Platform &platform,
                           uint32_t width,
                           uint32_t height)
        : Window(platform, width, height)
    {
        if (!glfwInit())
            throw std::runtime_error("GLFW couldn't be initialized.");

        glfwSetErrorCallback(ErrorCallback);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        const Options &options = platform.GetApplication().GetOptions();

        if (options.Contains("--width"))
            width = static_cast<uint32_t>(options.GetInt("--width"));

        if (options.Contains("--height"))
            width = static_cast<uint32_t>(options.GetInt("--height"));

        const char *name = platform.GetApplication().GetName().c_str();

        m_Handle = glfwCreateWindow(width, height, name, nullptr, nullptr);

        if (!m_Handle)
            throw std::runtime_error("Couldn't create GLFW window.");

        glfwSetWindowUserPointer(m_Handle, this);

        glfwSetWindowCloseCallback(m_Handle, WindowCloseCallback);
    }

    GlfwWindow::~GlfwWindow()
    {
        glfwTerminate();
    }

    void GlfwWindow::ProcessEvents()
    {
        glfwPollEvents();
    }

    VkSurfaceKHR GlfwWindow::CreateSurface(Instance &instance)
    {
        return nullptr;
    }

    bool GlfwWindow::ShouldClose() const
    {
        return glfwWindowShouldClose(m_Handle);
    }

    void GlfwWindow::Close()
    {
        glfwSetWindowShouldClose(m_Handle, GLFW_TRUE);
    }
}