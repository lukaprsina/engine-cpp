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
        std::cout << "Source: \t\t" << source_directory.generic_string() << std::endl;

        auto bin_directory = std::filesystem::path(ENG_BIN_DIRECTORY);
        std::cout << "Bin: \t\t\t" << bin_directory.generic_string() << std::endl;

        auto base_directory = std::filesystem::current_path();
        base_directory = std::filesystem::canonical(base_directory);
        std::cout << "Canonical Base: \t" << base_directory.generic_string() << std::endl;

#ifdef ENG_SHIPPING
        if (source_directory == base_directory)
        {
            std::cout << "From source" << std::endl;
            base_directory /= m_EngineName;
        }
        else
        {
            int i = 0;
            auto new_bin_dir = bin_directory;

            for (auto &folder : bin_directory)
            {
                new_bin_dir = new_bin_dir.parent_path();
                if (new_bin_dir.filename().generic_string() == m_EngineName)
                    break;

                i++;
            }

            int base_size = 0;
            for (auto &folder : base_directory)
                base_size++;

            auto new_base = std::filesystem::path();
            int base_folder_index = base_size - i - 1;
            int j = 0;
            for (auto &folder : base_directory)
            {
                if (j == base_folder_index)
                    break;
                else
                    new_base = new_base / folder;
                j++;
            }

            std::cout << new_base.generic_string() << std::endl;
            std::cout << "Not from source" << std::endl;
            base_directory = new_base;
        }
#else
        base_directory /= m_EngineName;
#endif

        std::cout << "Base: " << base_directory.generic_string() << std::endl;
        std::filesystem::current_path(base_directory);
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
        bool result = false;
        if (m_App)
            result = m_App->Prepare();

        for (auto &layer_pair : m_App->GetLayerStack().GetLayers())
        {
            Layer *layer = layer_pair.second.get();
            layer->OnAttach();
        }

        return result;
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