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

    class Platform
    {
    public:
        Platform() = default;
        virtual ~Platform() = default;

        virtual bool Initialize(std::unique_ptr<Application> &&app);
        virtual bool Prepare();
        virtual void MainLoop();
        void Run();
        virtual void Terminate(ExitCode code);
        virtual void Close() const;

        Window &GetWindow() const { return *m_Window; };
        Application &GetApp() const { return *m_App; };
        virtual const char *GetSurfaceExtension() = 0;

        static void SetArguments(const std::vector<std::string> &arguments) { m_Arguments = arguments; };
        std::vector<std::string> &GetArguments() { return m_Arguments; };

        static void SetExternalStorageDirectory(const std::string &directory) { m_ExternalStorageDirectory = directory; };
        std::string &GetExternalStorageDirectory() { return m_ExternalStorageDirectory; };

        static void SetTempDirectory(const std::string &directory) { m_TempDirectory = directory; };
        std::string &GetTempDirectory() { return m_TempDirectory; };

    protected:
        std::unique_ptr<Window> m_Window;
        std::unique_ptr<Application> m_App;
        Timer m_Timer;

        virtual void CreatePlatformWindow() = 0;

    private:
        static std::vector<std::string> m_Arguments;
        static std::string m_ExternalStorageDirectory;
        static std::string m_TempDirectory;
    };
}