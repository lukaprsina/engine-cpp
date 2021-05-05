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
        inline KeyCode TranslateKeyCode(int key)
        {
            static const std::unordered_map<int, KeyCode> key_map =
                {
                    {GLFW_KEY_SPACE, KeyCode::Space},
                    {GLFW_KEY_APOSTROPHE, KeyCode::Apostrophe},
                    {GLFW_KEY_COMMA, KeyCode::Comma},
                    {GLFW_KEY_MINUS, KeyCode::Minus},
                    {GLFW_KEY_PERIOD, KeyCode::Period},
                    {GLFW_KEY_SLASH, KeyCode::Slash},
                    {GLFW_KEY_0, KeyCode::_0},
                    {GLFW_KEY_1, KeyCode::_1},
                    {GLFW_KEY_2, KeyCode::_2},
                    {GLFW_KEY_3, KeyCode::_3},
                    {GLFW_KEY_4, KeyCode::_4},
                    {GLFW_KEY_5, KeyCode::_5},
                    {GLFW_KEY_6, KeyCode::_6},
                    {GLFW_KEY_7, KeyCode::_7},
                    {GLFW_KEY_8, KeyCode::_8},
                    {GLFW_KEY_9, KeyCode::_9},
                    {GLFW_KEY_SEMICOLON, KeyCode::Semicolon},
                    {GLFW_KEY_EQUAL, KeyCode::Equal},
                    {GLFW_KEY_A, KeyCode::A},
                    {GLFW_KEY_B, KeyCode::B},
                    {GLFW_KEY_C, KeyCode::C},
                    {GLFW_KEY_D, KeyCode::D},
                    {GLFW_KEY_E, KeyCode::E},
                    {GLFW_KEY_F, KeyCode::F},
                    {GLFW_KEY_G, KeyCode::G},
                    {GLFW_KEY_H, KeyCode::H},
                    {GLFW_KEY_I, KeyCode::I},
                    {GLFW_KEY_J, KeyCode::J},
                    {GLFW_KEY_K, KeyCode::K},
                    {GLFW_KEY_L, KeyCode::L},
                    {GLFW_KEY_M, KeyCode::M},
                    {GLFW_KEY_N, KeyCode::N},
                    {GLFW_KEY_O, KeyCode::O},
                    {GLFW_KEY_P, KeyCode::P},
                    {GLFW_KEY_Q, KeyCode::Q},
                    {GLFW_KEY_R, KeyCode::R},
                    {GLFW_KEY_S, KeyCode::S},
                    {GLFW_KEY_T, KeyCode::T},
                    {GLFW_KEY_U, KeyCode::U},
                    {GLFW_KEY_V, KeyCode::V},
                    {GLFW_KEY_W, KeyCode::W},
                    {GLFW_KEY_X, KeyCode::X},
                    {GLFW_KEY_Y, KeyCode::Y},
                    {GLFW_KEY_Z, KeyCode::Z},
                    {GLFW_KEY_LEFT_BRACKET, KeyCode::LeftBracket},
                    {GLFW_KEY_BACKSLASH, KeyCode::Backslash},
                    {GLFW_KEY_RIGHT_BRACKET, KeyCode::RightBracket},
                    {GLFW_KEY_GRAVE_ACCENT, KeyCode::GraveAccent},
                    {GLFW_KEY_ESCAPE, KeyCode::Escape},
                    {GLFW_KEY_ENTER, KeyCode::Enter},
                    {GLFW_KEY_TAB, KeyCode::Tab},
                    {GLFW_KEY_BACKSPACE, KeyCode::Backspace},
                    {GLFW_KEY_INSERT, KeyCode::Insert},
                    {GLFW_KEY_DELETE, KeyCode::DelKey},
                    {GLFW_KEY_RIGHT, KeyCode::Right},
                    {GLFW_KEY_LEFT, KeyCode::Left},
                    {GLFW_KEY_DOWN, KeyCode::Down},
                    {GLFW_KEY_UP, KeyCode::Up},
                    {GLFW_KEY_PAGE_UP, KeyCode::PageUp},
                    {GLFW_KEY_PAGE_DOWN, KeyCode::PageDown},
                    {GLFW_KEY_HOME, KeyCode::Home},
                    {GLFW_KEY_END, KeyCode::End},
                    {GLFW_KEY_CAPS_LOCK, KeyCode::CapsLock},
                    {GLFW_KEY_SCROLL_LOCK, KeyCode::ScrollLock},
                    {GLFW_KEY_NUM_LOCK, KeyCode::NumLock},
                    {GLFW_KEY_PRINT_SCREEN, KeyCode::PrintScreen},
                    {GLFW_KEY_PAUSE, KeyCode::Pause},
                    {GLFW_KEY_F1, KeyCode::F1},
                    {GLFW_KEY_F2, KeyCode::F2},
                    {GLFW_KEY_F3, KeyCode::F3},
                    {GLFW_KEY_F4, KeyCode::F4},
                    {GLFW_KEY_F5, KeyCode::F5},
                    {GLFW_KEY_F6, KeyCode::F6},
                    {GLFW_KEY_F7, KeyCode::F7},
                    {GLFW_KEY_F8, KeyCode::F8},
                    {GLFW_KEY_F9, KeyCode::F9},
                    {GLFW_KEY_F10, KeyCode::F10},
                    {GLFW_KEY_F11, KeyCode::F11},
                    {GLFW_KEY_F12, KeyCode::F12},
                    {GLFW_KEY_KP_0, KeyCode::KP_0},
                    {GLFW_KEY_KP_1, KeyCode::KP_1},
                    {GLFW_KEY_KP_2, KeyCode::KP_2},
                    {GLFW_KEY_KP_3, KeyCode::KP_3},
                    {GLFW_KEY_KP_4, KeyCode::KP_4},
                    {GLFW_KEY_KP_5, KeyCode::KP_5},
                    {GLFW_KEY_KP_6, KeyCode::KP_6},
                    {GLFW_KEY_KP_7, KeyCode::KP_7},
                    {GLFW_KEY_KP_8, KeyCode::KP_8},
                    {GLFW_KEY_KP_9, KeyCode::KP_9},
                    {GLFW_KEY_KP_DECIMAL, KeyCode::KP_Decimal},
                    {GLFW_KEY_KP_DIVIDE, KeyCode::KP_Divide},
                    {GLFW_KEY_KP_MULTIPLY, KeyCode::KP_Multiply},
                    {GLFW_KEY_KP_SUBTRACT, KeyCode::KP_Subtract},
                    {GLFW_KEY_KP_ADD, KeyCode::KP_Add},
                    {GLFW_KEY_KP_ENTER, KeyCode::KP_Enter},
                    {GLFW_KEY_KP_EQUAL, KeyCode::KP_Equal},
                    {GLFW_KEY_LEFT_SHIFT, KeyCode::LeftShift},
                    {GLFW_KEY_LEFT_CONTROL, KeyCode::LeftControl},
                    {GLFW_KEY_LEFT_ALT, KeyCode::LeftAlt},
                    {GLFW_KEY_RIGHT_SHIFT, KeyCode::RightShift},
                    {GLFW_KEY_RIGHT_CONTROL, KeyCode::RightControl},
                    {GLFW_KEY_RIGHT_ALT, KeyCode::RightAlt},
                };

            auto key_it = key_map.find(key);

            if (key_it == key_map.end())
            {
                return KeyCode::Unknown;
            }

            return key_it->second;
        }

        inline MouseButton TranslateMouseButton(int button)
        {
            if (button < GLFW_MOUSE_BUTTON_6)
            {
                return static_cast<MouseButton>(button);
            }

            return MouseButton::Unknown;
        }
    }
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

            KeyCode key_code = TranslateKeyCode(key);
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

        VkSurfaceKHR surface;
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