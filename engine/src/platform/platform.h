#pragma once

#include "core/application.h"
#include "core/timer.h"
#include "window/window.h"

namespace engine
{
    enum class ExitCode
    {
        Success = 0,     /* App prepare succeeded, it ran correctly and exited properly with no errors */
        UnableToRun = 1, /* App prepare failed, could not run */
        FatalError = 2   /* App encountered an unexpected error */
    };

    class Event;
    class Instance;

    class Platform
    {
    public:
        Platform(const std::string name, const std::vector<std::string> &arguments);
        virtual ~Platform() = default;

        virtual bool Initialize(std::unique_ptr<Application> &&app);
        virtual bool Prepare();
        virtual void MainLoop();
        void Run();
        virtual void Terminate(ExitCode code);

        virtual VkSurfaceKHR CreatePlatformWindow(Instance &instance) = 0;
        Window *GetWindow(VkSurfaceKHR surface) const { return m_Windows.at(surface).get(); }
        Application &GetApp() const { return *m_App; }
        virtual const char *GetSurfaceExtension() = 0;

        static void SetArguments(const std::vector<std::string> &arguments) { s_Arguments = arguments; };
        static std::vector<std::string> &GetArguments() { return s_Arguments; };

        static void SetSourceDirectory(const std::filesystem::path &directory) { s_SourceDirectory = directory; };
        static const std::filesystem::path &GetSourceDirectory() { return s_SourceDirectory; };

        static void SetExternalStorageDirectory(const std::filesystem::path &directory) { s_ExternalStorageDirectory = directory; };
        static const std::filesystem::path &GetExternalStorageDirectory() { return s_ExternalStorageDirectory; };

        static void SetTempDirectory(const std::filesystem::path &directory) { s_TempDirectory = directory; };
        static const std::filesystem::path &GetTempDirectory() { return s_TempDirectory; };

    protected:
        std::unordered_map<VkSurfaceKHR, std::unique_ptr<Window>> m_Windows{};
        std::unique_ptr<Application> m_App{};
        Timer m_Timer{};

    private:
        std::string m_EngineName = "engine";
        static std::vector<std::string> s_Arguments;
        static std::filesystem::path s_SourceDirectory;
        static std::filesystem::path s_ExternalStorageDirectory;
        static std::filesystem::path s_TempDirectory;
    };
}