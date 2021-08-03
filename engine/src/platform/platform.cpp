#include "platform/platform.h"

#include "events/event.h"
#include "core/application.h"

namespace engine
{
    std::vector<std::string> Platform::s_Arguments = {};
    std::fs::path Platform::s_ExternalStorageDirectory = {};
    std::fs::path Platform::s_TempDirectory = {};

    Platform::Platform(const std::vector<std::string> &arguments)
    {
        auto program_name = std::fs::path(arguments[0]);
        std::fs::current_path(program_name.parent_path().parent_path());

        std::vector<std::string> args = arguments;
        args.erase(args.begin());

        Platform::SetArguments(args);
    }

    bool Platform::Initialize(std::unique_ptr<Application> &&app)
    {
        ENG_ASSERT(app && "Application isn't valid!");
        m_App = std::move(app);

        bool is_headless = m_App->GetOptions().Contains("--headless");
        m_App->SetHeadless(is_headless);

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
        ENG_CORE_INFO("Starting the main loop.");
        while (!m_Window->ShouldClose())
        {
            Run();
            m_Window->ProcessEvents();
        }
    }

    void Platform::Run()
    {
        m_App->Step();
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