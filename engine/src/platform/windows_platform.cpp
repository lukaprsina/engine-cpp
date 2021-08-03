#include "platform/windows_platform.h"

#include "window/glfw_window.h"
#include "window/headless_window.h"
#include "events/event.h"

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

    WindowsPlatform::WindowsPlatform(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                                     PSTR lpCmdLine, INT nCmdShow)
        : Platform(GetArgs())
    {
        if (!AllocConsole())
        {
            throw std::runtime_error{"AllocConsole error"};
        }

        FILE *fp;
        freopen_s(&fp, "conin$", "r", stdin);
        freopen_s(&fp, "conout$", "w", stdout);
        freopen_s(&fp, "conout$", "w", stderr);

        Platform::SetTempDirectory(GetTempPathFromEnvironment());
    }

    bool WindowsPlatform::Initialize(std::unique_ptr<Application> &&app)
    {
        return Platform::Initialize(std::move(app)) && Platform::Prepare();
    }

    void WindowsPlatform::CreatePlatformWindow()
    {
        WindowSettings settings;

        if (m_App->IsHeadless())
        {
            m_Window = std::make_unique<HeadlessWindow>(*this, settings);
        }
        else
        {
            m_Window = std::make_unique<GlfwWindow>(*this, settings);
        }
    }

    const char *WindowsPlatform::GetSurfaceExtension()
    {
        return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    }
}