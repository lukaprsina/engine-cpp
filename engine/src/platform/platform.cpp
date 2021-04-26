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

        bool isHeadless = m_App->GetOptions().Contains("--headless");
        m_App->SetHeadless(isHeadless);

        CreatePlatformWindow();

        if (!m_Window)
            throw std::runtime_error("Can't create window!");
        else
            ENG_CORE_TRACE("Window created!");

        return true;
    }

    bool Platform::Prepare()
    {
        return true;
    }

    void Platform::MainLoop()
    {
        while (true)
        {
        }
    }

    void Platform::Terminate(ExitCode)
    {
    }

    void Platform::Close() const
    {
    }
}