#pragma once

#include "vulkan_api/instance.h"

namespace engine
{
    class Platform;
    class Event;
    class RenderContext;
    class Device;

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

        RenderContext &GetRenderContext()
        {
            ENG_ASSERT(m_RenderContext, "Render context is not valid");
            return *m_RenderContext;
        }

        void CreateRenderContext(Device &device,
                                 std::vector<VkPresentModeKHR> &present_mode_priority,
                                 std::vector<VkSurfaceFormatKHR> &surface_format_priority) {}
        void DeleteRenderContext() { m_RenderContext.reset(); }

        void SetSettings(WindowSettings &settings);
        WindowSettings GetSettings() { return m_Settings; }
        void SetEventCallback(const std::function<void(Event &)> &event_callback) { m_Settings.EventCallback = event_callback; }

    protected:
        Platform &m_Platform;
        WindowSettings m_Settings;
        WindowSettings m_WindowedSettings;
        bool m_Dirty{false};

        std::unique_ptr<RenderContext> m_RenderContext{};
    };
}