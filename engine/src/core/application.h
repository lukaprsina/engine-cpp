#pragma once

#include "core/options.h"
#include "events/application_event.h"

namespace engine
{
    class Application
    {
    public:
        Application();
        ~Application() = default;

        void OnEvent(Event &event);
        bool OnWindowClose(WindowCloseEvent &event);

        void SetName(const std::string &name) { m_Name = name; }
        std::string GetName() const { return m_Name; };
        void SetUsage(const std::string &usage) { m_Usage = usage; }

        void ParseOptions(std::vector<std::string> &arguments);
        Options GetOptions() const { return m_Options; };

        void SetHeadless(bool headless) { m_Headless = headless; };
        bool IsHeadless() const { return m_Headless; };

    private:
        std::string m_Name;
        std::string m_Usage;
        Options m_Options;
        bool m_Headless;
    };
}