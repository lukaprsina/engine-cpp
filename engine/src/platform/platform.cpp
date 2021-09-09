#include "platform/platform.h"

#include "events/event.h"
#include "core/application.h"
#include "window/input.h"
#include "engine_config.h"

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

        CreatePlatformWindow();

        if (!m_Window)
            throw std::runtime_error("Can't create window!");
        else
            ENG_CORE_INFO("Window created!");

        Input::m_WindowPointer = m_Window->GetNativeWindow();

        m_Window->SetEventCallback(std::bind(&Application::OnEvent, m_App.get(), std::placeholders::_1));

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
        while (!m_Window->ShouldClose())
        {
            if (!m_Window->GetSettings().minimized)
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

        spdlog::drop_all();
    }

    void Platform::Close() const
    {
        m_Window->Close();
    }
}