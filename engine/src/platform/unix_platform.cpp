#include "platform/unix_platform.h"

#include "window/glfw_window.h"
#include "window/headless_window.h"
#include "events/event.h"

#ifndef VK_MVK_MACOS_SURFACE_EXTENSION_NAME
#define VK_MVK_MACOS_SURFACE_EXTENSION_NAME "VK_MVK_macos_surface"
#endif

#ifndef VK_KHR_XCB_SURFACE_EXTENSION_NAME
#define VK_KHR_XCB_SURFACE_EXTENSION_NAME "VK_KHR_xcb_surface"
#endif

namespace engine
{
    namespace
    {
        inline const std::string GetTempPathFromEnvironment()
        {
            std::string temp_path = "/tmp/";

            if (const char *env_ptr = std::getenv("TMPDIR"))
            {
                temp_path = std::string(env_ptr) + "/";
            }

            return temp_path;
        }
    }

    UnixPlatform::UnixPlatform(const UnixType &type, int argc, char *argv[])
        : Platform(), m_Type(type)
    {
        Platform::SetArguments({argv + 1, argv + argc});
        Platform::SetTempDirectory(GetTempPathFromEnvironment());
    }

    bool UnixPlatform::Initialize(std::unique_ptr<Application> &&app)
    {
        return Platform::Initialize(std::move(app)) && Platform::Prepare();
    }

    void UnixPlatform::CreatePlatformWindow()
    {
        WindowSettings settings;

        if (m_App->IsHeadless())
        {
            m_Window = std::make_unique<HeadlessWindow>(*this, settings);
        }
        else
        {
            m_Window = std::make_unique<GlfwWindow>(*this, settings);
        }
    }

    const char *UnixPlatform::GetSurfaceExtension()
    {
        if (m_Type == UnixType::Mac)
        {
            return VK_MVK_MACOS_SURFACE_EXTENSION_NAME;
        }
        else
        {
            return VK_KHR_XCB_SURFACE_EXTENSION_NAME;
        }
    }
}