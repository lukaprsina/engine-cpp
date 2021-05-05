#pragma once

#include "vulkan_api/instance/instance.h"

namespace engine
{
    class Platform;
    class Event;

    struct WindowSettings
    {
        std::string title = "";
        uint32_t width = 1280;
        uint32_t height = 720;
        bool focused = true;

        std::function<void(Event &)> EventCallback = nullptr;
    };

    class Window
    {
    public:
        Window(Platform &platform, WindowSettings &settings);
        virtual ~Window() = default;

        virtual void ProcessEvents() {}
        virtual VkSurfaceKHR CreateSurface(Instance &instance) = 0;
        virtual bool ShouldClose() const = 0;
        virtual void Close() = 0;

        void SetSettings(WindowSettings &settings) { m_Settings = settings; }
        WindowSettings GetSettings() { return m_Settings; }
        void SetEventCallback(const std::function<void(Event &)> &event_callback) { m_Settings.EventCallback = event_callback; }

    protected:
        Platform &m_Platform;
        WindowSettings m_Settings;
    };
}