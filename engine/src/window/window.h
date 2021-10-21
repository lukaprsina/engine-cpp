#pragma once

#include "vulkan_api/instance.h"
#include "window/input.h"

namespace engine
{
    class Platform;
    class Event;
    class Device;
    class PhysicalDevice;
    class RenderContext;
    class Scene;
    class Layer;
    class CommandBuffer;
    class RenderTarget;

    struct WindowSettings
    {
        std::string title{};
        int32_t width = 1280;
        int32_t height = 720;
        int32_t posx = 0;
        int32_t posy = 0;
        bool focused = true;
        bool fullscreen = false;
        bool minimized = false;
        bool decorated = true;
        bool floating = false;

        std::function<void(Event &)> EventCallback = nullptr;
    };

    class Window
    {
    public:
        Window(Platform &platform, WindowSettings &settings);
        virtual ~Window();

        virtual void ProcessEvents(){};
        virtual VkSurfaceKHR CreateSurface(Instance &instance, PhysicalDevice &physical_device) = 0;
        void Draw();
        void Render(CommandBuffer &command_buffer, RenderTarget &render_target, Layer *layer);
        void SetViewportAndScissor(CommandBuffer &command_buffer, const VkExtent2D &extent) const;
        virtual bool ShouldClose() const = 0;
        virtual void Destroy() = 0;
        virtual void Close() = 0;
        virtual void *GetNativeWindow() = 0;
        void OnEvent(Event &event);

        void AddScene(Scene *scene);
        void AddLayer(Layer *layer);

        void SetSettings(WindowSettings &settings);
        WindowSettings GetSettings() { return m_Settings; }
        VkSurfaceKHR GetSurface() { return m_Surface; }
        std::vector<Layer *> &GetLayers() { return m_Layers; }
        Input &GetInput() { return m_Input; }
        void SetEventCallback(const std::function<void(Event &)> &event_callback) { m_Settings.EventCallback = event_callback; }

        RenderContext &CreateRenderContext(Device &device,
                                           std::vector<VkPresentModeKHR> &present_mode_priority,
                                           std::vector<VkSurfaceFormatKHR> &surface_format_priority);
        RenderContext &GetRenderContext();
        void DeleteRenderContext();
        bool IsRenderContextValid() { return static_cast<bool>(m_RenderContext); }

    protected:
        Platform &m_Platform;
        WindowSettings m_Settings;
        WindowSettings m_WindowedSettings;
        VkSurfaceKHR m_Surface{VK_NULL_HANDLE};
        Input m_Input;
        std::unique_ptr<RenderContext> m_RenderContext{};
        std::vector<Scene *> m_Scenes{};
        std::vector<Layer *> m_Layers{};
        std::vector<CommandBuffer *> m_CommandBuffers{};
        bool m_Dirty{false};
    };
}