#pragma once

#include "core/options.h"
#include "core/timer.h"

namespace engine
{
    class Platform;
    class WindowCloseEvent;
    class WindowResizeEvent;
    class Event;
    class Instance;
    class Device;
    class RenderContext;
    class Scene;
    class RenderTarget;
    class RenderPipeline;
    class CommandBuffer;

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
        bool OnResize(WindowResizeEvent &event);
        bool Prepare();
        void Step();
        void Update(float delta_time);
        void UpdateScene(float delta_time);
        void Draw(CommandBuffer &command_buffer);
        void Finish();

        void LoadScene(const std::string &path);

        void SetViewportAndScissor(CommandBuffer &command_buffer, const VkExtent2D &extent) const;

        void SetName(const std::string &name)
        {
            m_Name = name;
        }
        std::string GetName() const { return m_Name; };

        void SetUsage(const std::string &usage) { m_Usage = usage; }
        std::string GetUsage() { return m_Usage; }

        void ParseOptions(std::vector<std::string> &arguments);
        Options GetOptions() const { return m_Options; };

        void SetHeadless(bool headless) { m_Headless = headless; };
        bool IsHeadless() const { return m_Headless; };

        RenderContext &GetRenderContext()
        {
            ENG_ASSERT(m_RenderContext, "Render context is not valid");
            return *m_RenderContext;
        }

        void AddInstanceExtension(const char *extension, bool optional = false) { m_InstanceExtensions[extension] = optional; }
        const std::unordered_map<const char *, bool> GetInstanceExtensions() { return m_InstanceExtensions; }
        void AddDeviceExtension(const char *extension, bool optional = false) { m_DeviceExtensions[extension] = optional; }
        const std::unordered_map<const char *, bool> GetDeviceExtensions() { return m_DeviceExtensions; }

    private:
        Platform *m_Platform;
        std::string m_Name{};
        std::string m_Usage{};
        Options m_Options{};
        bool m_Headless{false};

        float m_Fps{0.0f};
        float m_FrameTime{0.0f};
        uint32_t m_FrameCount{0};
        uint32_t m_LastFrameCount{0};
        Timer m_Timer{};

        std::unique_ptr<Instance> m_Instance{};
        std::unique_ptr<Device> m_Device{};
        std::unique_ptr<Scene> m_Scene{};
        std::unique_ptr<RenderContext> m_RenderContext{};
        std::unique_ptr<RenderPipeline> m_RenderPipeline{};

        std::unordered_map<const char *, bool> m_DeviceExtensions{};
        std::unordered_map<const char *, bool> m_InstanceExtensions{};
        std::vector<const char *> m_ValidationLayers{};

        VkSurfaceKHR m_Surface{VK_NULL_HANDLE};
    };
}