#include "platform/platform.h"

#include "events/event.h"
#include "core/application.h"

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
            ENG_CORE_INFO("Window created!");

        m_Window->SetEventCallback(std::bind(&Application::OnEvent, m_App.get(), std::placeholders::_1));

        return true;
    }

    bool Platform::Prepare()
    {
        if (m_App)
        {
            return m_App->Prepare();
        }

        return false;
    }

    void Platform::MainLoop()
    {
        while (!m_Window->ShouldClose())
        {
            Run();
        }
    }

    void Platform::Run()
    {
        m_Window->ProcessEvents();
    }

    void Platform::Terminate(ExitCode)
    {
        if (m_App)
            m_App->Finish();

        m_App.reset();
        m_Window.reset();
    }

    void Platform::Close() const
    {
        m_Window->Close();
    }
}