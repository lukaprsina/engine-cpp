#include "platform/platform.h"
#include "common/base_common.h"

namespace engine
{
    std::vector<std::string> Platform::m_Arguments = {};
    std::string Platform::m_ExternalStorageDirectory = "";
    std::string Platform::m_TempDirectory = "";

    bool Platform::Initialize(std::unique_ptr<Application> &&app)
    {
        ENG_ASSERT(app && "Application isn't valid!");
        m_App = std::move(app);
        CreatePlatformWindow();
        return true;
    }

    bool Platform::Prepare()
    {
        return true;
    }

    void Platform::MainLoop()
    {
    }

    void Platform::Terminate(ExitCode)
    {
    }

    void Platform::Close() const
    {
    }
}