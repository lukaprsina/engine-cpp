#include "window/window.h"

#include "platform/platform.h"

namespace engine
{
    Window::Window(Platform &platform, WindowSettings &settings)
        : m_Platform(platform), m_Settings(settings)
    {
    }
}