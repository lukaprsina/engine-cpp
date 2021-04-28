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
        }

        void WindowSizeCallback(GLFWwindow *window, int width, int height)
        {
            WindowSettings &data = *(WindowSettings *)glfwGetWindowUserPointer(window);
            data.width = width;
            data.height = height;

            WindowResizeEvent event(width, height);
            data.eventCallback(event);
        }

        void WindowFocusCallback(GLFWwindow *window, int focused)
        {
            WindowSettings &data = *(WindowSettings *)glfwGetWindowUserPointer(window);

            WindowFocusedEvent event(focused);
            data.eventCallback(event);
        }

        void WindowPositionCallback(GLFWwindow *window, int xPos, int yPos)
        {
            WindowSettings &data = *(WindowSettings *)glfwGetWindowUserPointer(window);

            WindowMovedEvent event(xPos, yPos);
            data.eventCallback(event);
        }

        void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
        {
        }

        void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
        {
        }

        void ScrollCallback(GLFWwindow *window, double xOffset, double yOffset)
        {
            WindowSettings &data = *(WindowSettings *)glfwGetWindowUserPointer(window);

            MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
            data.eventCallback(event);
        }

        void CursorPositionCallback(GLFWwindow *window, double xPos, double yPos)
        {
            WindowSettings &data = *(WindowSettings *)glfwGetWindowUserPointer(window);

            MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
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
        glfwSetWindowFocusCallback(m_Handle, WindowFocusCallback);
        glfwSetWindowPosCallback(m_Handle, WindowPositionCallback);
        // glfwSetKeyCallback(m_Handle, KeyCallback);
        // glfwSetMouseButtonCallback(m_Handle, MouseButtonCallback);
        glfwSetScrollCallback(m_Handle, ScrollCallback);
        glfwSetCursorPosCallback(m_Handle, CursorPositionCallback);
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