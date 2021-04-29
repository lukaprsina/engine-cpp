#pragma once

#include "core/options.h"
#include "vulkan_api/instance/instance.h"

namespace engine
{
    class Platform;
    class WindowCloseEvent;
    class Event;

    class Application
    {
    public:
        Application(Platform *platform);
        Application() = delete;
        Application(const Application &) = default;
        Application(Application &&) = delete;
        ~Application();
        Application &operator=(const Application &) = delete;
        Application &operator=(Application &&) = delete;

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

        void AddInstanceExtension(const char *extension, bool optional = false) { m_InstanceExtensions[extension] = optional; }
        const std::unordered_map<const char *, bool> GetInstanceExtensions() { return m_InstanceExtensions; }
        void AddDeviceExtension(const char *extension, bool optional = false) { m_DeviceExtensions[extension] = optional; }
        const std::unordered_map<const char *, bool> GetDeviceExtensions() { return m_DeviceExtensions; }

    private:
        Platform *m_Platform;
        std::string m_Name;
        std::string m_Usage;
        Options m_Options;
        bool m_Headless;

        std::unique_ptr<Instance> m_Instance;

        std::unordered_map<const char *, bool> m_DeviceExtensions;
        std::unordered_map<const char *, bool> m_InstanceExtensions;
        std::vector<const char *> m_ValidationLayers;
    };
}