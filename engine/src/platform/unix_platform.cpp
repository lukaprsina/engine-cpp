#include "platform/unix_platform.h"

#include "window/glfw_window.h"
#include "window/headless_window.h"
#include "events/event.h"
#include "platform/filesystem.h"

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
        inline std::filesystem::path GetTempPathFromEnvironment()
        {
            std::filesystem::path temp_path = "/tmp/";

            if (const char *env_ptr = std::getenv("TMPDIR"))
                temp_path = std::filesystem::path(env_ptr);

            return temp_path;
        }

        inline std::filesystem::path GetRootFolder()
        {
            auto working_directory = fs::path::Get(fs::path::Type::WorkingDirectory);
            return working_directory.parent_path();
        }
    }

    UnixPlatform::UnixPlatform(const UnixType &type, int argc, char *argv[])
        : Platform(std::vector<std::string>(argv, argv + argc)),
          m_Type(type)
    {
        Platform::SetTempDirectory(GetTempPathFromEnvironment());
        Platform::SetExternalStorageDirectory(GetRootFolder());
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
            return VK_MVK_MACOS_SURFACE_EXTENSION_NAME;

        else
            return VK_KHR_XCB_SURFACE_EXTENSION_NAME;
    }
}