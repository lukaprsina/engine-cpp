#pragma once

#include "core/options.h"

namespace engine
{
    class Platform;
    class WindowCloseEvent;
    class Event;

    class Application
    {
    public:
        Application(Platform *platform);
        ~Application() = default;

        void OnEvent(Event &event);
        bool OnWindowClose(WindowCloseEvent &event);
        bool Prepare();
        void Finish();

        void SetName(const std::string &name) { m_Name = name; }
        std::string GetName() const { return m_Name; };

        void SetUsage(const std::string &usage) { m_Usage = usage; }
        std::string GetUsage() { return m_Usage; }

        void ParseOptions(std::vector<std::string> &arguments);
        Options GetOptions() const { return m_Options; };

        void SetHeadless(bool headless) { m_Headless = headless; };
        bool IsHeadless() const { return m_Headless; };

    private:
        Platform *m_Platform;
        std::string m_Name;
        std::string m_Usage;
        Options m_Options;
        bool m_Headless;
    };
}