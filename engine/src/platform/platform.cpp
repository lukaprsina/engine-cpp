#include "platform/platform.h"

#include "events/event.h"
#include "core/application.h"
#include "window/input.h"
#include "core/layer_stack.h"

namespace engine
{
    std::vector<std::string> Platform::s_Arguments = {};
    std::filesystem::path Platform::s_SourceDirectory = {};
    std::filesystem::path Platform::s_ExternalStorageDirectory = {};
    std::filesystem::path Platform::s_TempDirectory = {};

    Platform::Platform(const std::string name, const std::vector<std::string> &arguments)
        : m_Name(name)
    {
        Platform::SetArguments(arguments);
    }

    void Platform::ConfigurePaths()
    {
        auto source_directory = std::filesystem::path(ENG_BASE_DIRECTORY);
        std::cout << "Source: " << source_directory.generic_string() << std::endl;

        auto base_directory = std::filesystem::current_path();
        std::cout << "Base: " << base_directory.generic_string() << std::endl;

        base_directory = std::filesystem::canonical(base_directory);
        std::cout << "Base: " << base_directory.generic_string() << std::endl;

#ifdef ENG_SHIPPING
        if (source_directory == base_directory)
        {
            std::cout << "From source" << std::endl;
            base_directory /= m_EngineName;
        }
        else
        {
            int i = 0;
            auto new_base = base_directory;
            for (auto &folder : base_directory)
            {
                new_base = new_base.parent_path();
                if (new_base.filename().generic_string() == m_EngineName)
                    break;

                i++;
            }

            int source_size = 0;
            for (auto &folder : source_directory)
                source_size++;

            auto new_source = std::filesystem::path();
            int source_folder_index = source_size - i + 1;
            int j = 0;
            for (auto &folder : source_directory)
            {
                if (j == source_folder_index)
                    break;
                else
                    new_source = new_source / folder;
                j++;
            }

            std::cout << new_source.generic_string() << std::endl;
            std::cout << "Not from source" << std::endl;

            base_directory = new_source / m_EngineName;
        }
#else
        base_directory /= m_EngineName;
#endif

        std::cout << "Base: " << base_directory.generic_string() << std::endl;

        Platform::SetSourceDirectory(base_directory);
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

        while (!ShouldClose())
        {
            m_App->Step();

            for (auto &window_pair : m_Windows)
            {
                auto &window = window_pair.second;
                window->Draw();
                window->ProcessEvents();
            }
        }
    }

    bool Platform::ShouldClose()
    {
        bool should_continue = false;

        for (auto &window_pair : m_Windows)
        {
            auto &window = window_pair.second;
            if (window->ShouldClose())
            {
                window->Close();
                m_ClosedWindows.emplace_back(window_pair.first);
            }

            else
                should_continue = true;
        }

        for (void *window_handle : m_ClosedWindows)
            m_Windows.erase(window_handle);

        m_ClosedWindows.clear();

        return !should_continue;
    }

    void Platform::Terminate(ExitCode)
    {
        if (m_App)
            m_App->Finish();

        m_App.reset();

        m_Windows.clear();

        spdlog::drop_all();
    }
}