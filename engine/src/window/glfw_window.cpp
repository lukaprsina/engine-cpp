#include "window/glfw_window.h"

#include "platform/platform.h"
#include "events/application_event.h"
#include "events/key_event.h"
#include "events/mouse_event.h"

ENG_DISABLE_WARNINGS()
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
ENG_ENABLE_WARNINGS()

namespace engine
{
    namespace
    {
        void ErrorCallback(int error, const char *description)
        {
            ENG_CORE_INFO("GLFW Error (code {}): {}", error, description);
        }

        void WindowCloseCallback(GLFWwindow *window)
        {
            WindowSettings &data = *(WindowSettings *)glfwGetWindowUserPointer(window);
            WindowCloseEvent event;
            data.EventCallback(event);
        }

        void WindowSizeCallback(GLFWwindow *window, int width, int height)
        {
            WindowSettings &data = *(WindowSettings *)glfwGetWindowUserPointer(window);
            data.width = width;
            data.height = height;

            WindowResizeEvent event(width, height);
            data.EventCallback(event);
        }

        void WindowFocusCallback(GLFWwindow *window, int focused)
        {
            WindowSettings &data = *(WindowSettings *)glfwGetWindowUserPointer(window);

            WindowFocusedEvent event(focused);
            data.EventCallback(event);
        }

        void WindowPositionCallback(GLFWwindow *window, int xPos, int yPos)
        {
            WindowSettings &data = *(WindowSettings *)glfwGetWindowUserPointer(window);

            WindowMovedEvent event(xPos, yPos);
            data.EventCallback(event);
        }

        void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            WindowSettings &data = *(WindowSettings *)glfwGetWindowUserPointer(window);
            static uint32_t repeat_count = 0;

            switch (action)
            {
            case GLFW_PRESS:
            {
                repeat_count = 0;
                KeyPressedEvent event(key, 0);
                data.EventCallback(event);
                break;
            }

            case GLFW_RELEASE:
            {
                KeyReleasedEvent event(key);
                data.EventCallback(event);
                break;
            }

            case GLFW_REPEAT:
            {
                repeat_count++;
                KeyPressedEvent event(key, repeat_count);
                data.EventCallback(event);
                break;
            }
            }
        }

        void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
        {
            WindowSettings &data = *(WindowSettings *)glfwGetWindowUserPointer(window);

            switch (action)
            {
            case GLFW_PRESS:
            {
                MouseButtonPressedEvent event(button);
                data.EventCallback(event);
                break;
            }
            case GLFW_RELEASE:
            {
                MouseButtonReleasedEvent event(button);
                data.EventCallback(event);
                break;
            }
            }
        }

        void ScrollCallback(GLFWwindow *window, double xOffset, double yOffset)
        {
            WindowSettings &data = *(WindowSettings *)glfwGetWindowUserPointer(window);

            MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
            data.EventCallback(event);
        }

        void CursorPositionCallback(GLFWwindow *window, double xPos, double yPos)
        {
            WindowSettings &data = *(WindowSettings *)glfwGetWindowUserPointer(window);

            MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
            data.EventCallback(event);
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

        SetSettings(settings);

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
        glfwSetKeyCallback(m_Handle, KeyCallback);
        glfwSetMouseButtonCallback(m_Handle, MouseButtonCallback);
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
        ENG_ASSERT(instance.GetHandle() && "Create an instance before calling CreateSurface.");
        ENG_ASSERT(m_Handle && "Create a window before calling CreateSurface.");

        if (instance.GetHandle() == VK_NULL_HANDLE || !m_Handle)
            return VK_NULL_HANDLE;

        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkResult result = glfwCreateWindowSurface(instance.GetHandle(), m_Handle, nullptr, &surface);

        if (result != VK_SUCCESS)
            return VK_NULL_HANDLE;

        return surface;
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