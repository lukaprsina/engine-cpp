#pragma once

#include "vulkan_api/instance.h"
#include "window/input.h"

namespace engine
{
    class Platform;
    class Event;
    class RenderContext;

    struct WindowSettings
    {
        std::string title = "";
        int32_t width = 1280;
        int32_t height = 720;
        int32_t posx = 0;
        int32_t posy = 0;
        bool focused = true;
        bool fullscreen = false;
        bool minimized = false;

        std::function<void(Event &)> EventCallback = nullptr;
    };

    class Window
    {
    public:
        Window(Platform &platform, WindowSettings &settings);
        virtual ~Window() = default;

        virtual void ProcessEvents(){};
        virtual VkSurfaceKHR CreateSurface(Instance &instance) = 0;
        virtual bool ShouldClose() const = 0;
        virtual void Close() = 0;
        virtual void *GetNativeWindow() = 0;

        void SetSettings(WindowSettings &settings);
        WindowSettings GetSettings() { return m_Settings; }
        Input &GetInput() { return m_Input; }
        void SetEventCallback(const std::function<void(Event &)> &event_callback) { m_Settings.EventCallback = event_callback; }

    protected:
        Platform &m_Platform;
        WindowSettings m_Settings;
        WindowSettings m_WindowedSettings;

        Input m_Input;
        std::unique_ptr<RenderContext> m_RenderContext{};

        bool m_Dirty{false};
    };
}