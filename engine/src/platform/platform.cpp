#include "platform/platform.h"

#include "events/event.h"
#include "core/application.h"
#include "window/input.h"
#include "window/window.h"

#include <filesystem>

namespace engine
{
    std::vector<std::string> Platform::s_Arguments = {};
    std::filesystem::path Platform::s_SourceDirectory = {};
    std::filesystem::path Platform::s_ExternalStorageDirectory = {};
    std::filesystem::path Platform::s_TempDirectory = {};

    Platform::Platform(const std::string name, const std::vector<std::string> &arguments)
    {
        auto source_directory = std::filesystem::canonical(ENG_BASE_DIRECTORY);

        Platform::SetSourceDirectory(source_directory / m_EngineName);

        Platform::SetArguments(arguments);
    }

    bool Platform::Initialize(std::unique_ptr<Application> &&app)
    {
        ENG_ASSERT(app && "Application isn't valid!");
        m_App = std::move(app);

        bool is_headless = m_App->GetOptions().Contains("--headless");
        m_App->SetHeadless(is_headless);

        return true;
    }

    bool Platform::Prepare()
    {
        if (m_App)
            return m_App->Prepare();

        return false;
    }

    void Platform::MainLoop()
    {
        ENG_CORE_INFO("Starting the main loop.");
        auto &window = m_Windows.at(m_App->GetSurface());

        while (window->ShouldClose())
        {
            if (!window->GetSettings().minimized)
                Run();
            window->ProcessEvents();
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
        m_Windows.at(m_App->GetSurface()).reset();

        spdlog::drop_all();
    }
}