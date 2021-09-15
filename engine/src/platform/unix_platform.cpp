#include "platform/unix_platform.h"

#include "window/glfw_window.h"
#include "window/input.h"
#include "window/headless_window.h"
#include "events/event.h"
#include "platform/filesystem.h"

#ifndef VK_MVK_MACOS_SURFACE_EXTENSION_NAME
#define VK_MVK_MACOS_SURFACE_EXTENSION_NAME "VK_MVK_macos_surface"
#endif

#ifndef VK_KHR_XCB_SURFACE_EXTENSION_NAME
#define VK_KHR_XCB_SURFACE_EXTENSION_NAME "VK_KHR_xcb_surface"
#endif

#ifndef VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#define VK_KHR_XLIB_SURFACE_EXTENSION_NAME "VK_KHR_xlib_surface"
#endif

#ifndef VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
#define VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME "VK_KHR_wayland_surface"
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
        : Platform(argv[0], std::vector<std::string>(argv + 1, argv + argc)),
          m_Type(type)
    {
        Platform::SetTempDirectory(GetTempPathFromEnvironment());
        Platform::SetExternalStorageDirectory(GetRootFolder());
    }

    bool UnixPlatform::Initialize(std::unique_ptr<Application> &&app)
    {
        return Platform::Initialize(std::move(app)) && Platform::Prepare();
    }

    VkSurfaceKHR UnixPlatform::CreatePlatformWindow(Instance &instance)
    {
        WindowSettings settings;
        VkSurfaceKHR surface;

        {
            std::unique_ptr<Window> window;

            if (m_App->IsHeadless())
            {
                window = std::make_unique<HeadlessWindow>(*this, settings, surface);
            }

            else
            {
                GlfwWindow::Init();
                window = std::make_unique<GlfwWindow>(*this, settings, instance, surface);
            }

            m_Windows.emplace(surface, std::move(window));
        }

        if (!m_Windows.at(surface)->GetNativeWindow())
            throw std::runtime_error("Can't create window!");
        else
            ENG_CORE_INFO("Window created!");

        Input::m_WindowPointer = m_Windows.at(surface)->GetNativeWindow();
        m_Windows.at(surface)->SetEventCallback(std::bind(&Application::OnEvent, m_App.get(), std::placeholders::_1));

        return surface;
    }

    const char *UnixPlatform::GetSurfaceExtension()
    {
        if (m_Type == UnixType::Mac)
            return VK_MVK_MACOS_SURFACE_EXTENSION_NAME;

#if defined(VK_USE_PLATFORM_XCB_KHR)
        return VK_KHR_XCB_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
        return VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        return VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
#else
        ENG_ASSERT(surface_extesion, "Platform not supported, no surface extension available");
        return "";
#endif
    }
}