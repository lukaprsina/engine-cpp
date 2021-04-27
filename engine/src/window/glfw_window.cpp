#include "window/glfw_window.h"

#include "platform/platform.h"
#include "events/application_event.h"
#include "events/key_event.h"
#include "events/mouse_event.h"

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
            WindowSettings &data = *(WindowSettings *)glfwGetWindowUserPointer(window);
            WindowCloseEvent event;
            data.eventCallback(event);
            // glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        void WindowSizeCallback(GLFWwindow *window, int width, int height)
        {
            WindowSettings &data = *(WindowSettings *)glfwGetWindowUserPointer(window);
            data.width = width;
            data.height = height;

            WindowResizeEvent event(width, height);
            data.eventCallback(event);
        }
    }

    GlfwWindow::GlfwWindow(Platform &platform,
                           WindowSettings &settings)
        : Window(platform, settings)
    {
        if (!glfwInit())
            throw std::runtime_error("GLFW couldn't be initialized.");

        glfwSetErrorCallback(ErrorCallback);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        const Options &options = platform.GetApp().GetOptions();

        if (options.Contains("--width"))
            settings.width = static_cast<uint32_t>(options.GetInt("--width"));

        if (options.Contains("--height"))
            settings.height = static_cast<uint32_t>(options.GetInt("--height"));

        if (settings.title.empty())
            settings.title = platform.GetApp().GetName();

        Window::SetSettings(settings);

        m_Handle = glfwCreateWindow(settings.width,
                                    settings.height,
                                    settings.title.c_str(), nullptr, nullptr);

        if (!m_Handle)
            throw std::runtime_error("Couldn't create GLFW window.");

        glfwSetWindowUserPointer(m_Handle, &m_Settings);

        glfwSetWindowCloseCallback(m_Handle, WindowCloseCallback);
        glfwSetWindowSizeCallback(m_Handle, WindowSizeCallback);
    }

    GlfwWindow::~GlfwWindow()
    {
        if (m_Handle)
            glfwDestroyWindow(m_Handle);

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