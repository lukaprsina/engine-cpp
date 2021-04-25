#pragma once

namespace engine
{
    class Application
    {
    public:
        Application();
        ~Application();

        bool IsHeadless() { return m_Headless; };

    private:
        static std::string m_Usage;
        bool m_Headless;
    };
}