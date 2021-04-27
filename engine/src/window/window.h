#pragma once

#include "common/vulkan_common.h"

namespace engine
{
    class Platform;
    class Event;
    class Instance;

    struct WindowSettings
    {
        std::string title = "";
        uint32_t width = 1280;
        uint32_t height = 720;
        bool focused = true;

        std::function<void(Event &)> eventCallback;
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
        void SetEventCallback(const std::function<void(Event &)> &eventCallback) { m_Settings.eventCallback = eventCallback; }

    protected:
        Platform &m_Platform;

    private:
        WindowSettings m_Settings;
    };
}