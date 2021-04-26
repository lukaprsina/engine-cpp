#include "platform/unix_platform.h"

#include "window/glfw_window.h"
#include "window/headless_window.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

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
        if (m_App->IsHeadless())
        {
            m_Window = std::make_unique<HeadlessWindow>(*this);
        }
        else
        {
            m_Window = std::make_unique<GlfwWindow>(*this);
        }
    }
}