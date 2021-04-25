#include "window/window.h"

namespace engine
{
    Window::Window(Platform &platform, uint32_t width, uint32_t height)
        : m_Platform(platform), m_Width(width), m_Height(height)
    {
    }
}