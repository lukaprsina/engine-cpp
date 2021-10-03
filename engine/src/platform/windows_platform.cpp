#include "platform/windows_platform.h"

#include "window/glfw_window.h"
#include "window/headless_window.h"
#include "events/event.h"
#include "platform/filesystem.h"

#include <stdio.h>
#include <iostream>

#include <windows.h>
#include <shellapi.h>

namespace engine
{
    namespace
    {
        inline const std::string GetTempPathFromEnvironment()
        {
            std::string temp_path = "temp/";

            TCHAR temp_buffer[MAX_PATH];
            DWORD temp_path_ret = GetTempPath(MAX_PATH, temp_buffer);
            if (temp_path_ret > MAX_PATH || temp_path_ret == 0)
            {
                temp_path = "temp/";
            }
            else
            {
                temp_path = std::string(temp_buffer) + "/";
            }

            return temp_path;
        }

        std::string wstr_to_str(const std::wstring &wstr)
        {
            if (wstr.empty())
            {
                return {};
            }

            auto wstr_len = static_cast<int>(wstr.size());
            auto str_len = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], wstr_len, NULL, 0, NULL, NULL);

            std::string str(str_len, 0);
            WideCharToMultiByte(CP_UTF8, 0, &wstr[0], wstr_len, &str[0], str_len, NULL, NULL);

            return str;
        }

        inline std::vector<std::string> GetArgs()
        {
            LPWSTR *argv;
            int argc;

            argv = CommandLineToArgvW(GetCommandLineW(), &argc);

            // Ignore the first argument containing the application full path
            std::vector<std::wstring> arg_strings(argv + 1, argv + argc);
            std::vector<std::string> args;

            for (auto &arg : arg_strings)
            {
                args.push_back(wstr_to_str(arg));
            }

            return args;
        }

        inline std::string GetName()
        {
            TCHAR szFileName[MAX_PATH];
            GetModuleFileName(NULL, szFileName, MAX_PATH);

            return std::string{szFileName};
        }

        inline std::filesystem::path GetRootFolder()
        {
            auto working_directory = fs::path::Get(fs::path::Type::WorkingDirectory);
            return working_directory.parent_path();
        }
    }

    WindowsPlatform::WindowsPlatform(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                                     PSTR lpCmdLine, INT nCmdShow)
        : Platform(GetName(), GetArgs())
    {
        auto args = GetArgs();
        if (!AllocConsole())
        {
            throw std::runtime_error{"AllocConsole error"};
        }

        FILE *fp;
        freopen_s(&fp, "conin$", "r", stdin);
        freopen_s(&fp, "conout$", "w", stdout);
        freopen_s(&fp, "conout$", "w", stderr);

        ConfigurePaths();

        Platform::SetTempDirectory(GetTempPathFromEnvironment());
        Platform::SetExternalStorageDirectory(GetRootFolder());
    }

    bool WindowsPlatform::Initialize(std::unique_ptr<Application> &&app)
    {
        return Platform::Initialize(std::move(app)) && Platform::Prepare();
    }

    Window *WindowsPlatform::CreatePlatformWindow()
    {
        WindowSettings settings;
        void *handle;

        {
            std::unique_ptr<Window> window;

            if (m_App->IsHeadless())
                window = std::make_unique<HeadlessWindow>(*this, settings);

            else
            {
                GlfwWindow::Init();
                window = std::make_unique<GlfwWindow>(*this, settings);
            }

            handle = window->GetNativeWindow();
            m_Windows.emplace(handle, std::move(window));
        }

        auto &window = m_Windows.at(handle);

        if (!window)
            throw std::runtime_error("Can't create window!");
        else
            ENG_CORE_INFO("Window created!");

        // TODO: move to window
        window->SetEventCallback(std::bind(&Window::OnEvent, window.get(), std::placeholders::_1));

        return window.get();
    }

    const char *WindowsPlatform::GetSurfaceExtension()
    {
        return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    }
}