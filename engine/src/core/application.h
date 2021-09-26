#pragma once

#include "core/options.h"
#include "core/timer.h"
#include "core/layer_stack.h"

namespace engine
{
    class Window;
    class Platform;
    class WindowCloseEvent;
    class WindowResizeEvent;
    class KeyPressedEvent;
    class Event;
    class Instance;
    class Device;
    class Scene;
    class RenderTarget;
    class RenderPipeline;
    class CommandBuffer;
    class Gui;

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
        bool OnKeyPressed(KeyPressedEvent &event);

        virtual bool Prepare();
        void Step();
        void Update(float delta_time);

        void Draw(Window *window);
        void Finish();
        bool ShouldClose() { return false; }

        Scene *LoadScene(std::string name);
        void SetViewportAndScissor(CommandBuffer &command_buffer, const VkExtent2D &extent) const;
        void SetName(const std::string &name) { m_Name = name; }
        std::string GetName() const { return m_Name; }

        Platform &GetPlatform() { return *m_Platform; }
        Instance &GetInstance() { return *m_Instance; }
        Device &GetDevice() { return *m_Device; }
        LayerStack &GetLayerStack() { return m_LayerStack; }
        std::vector<Scene *> &GetScenes() { return m_Scenes; }

        void SetUsage(const std::string &usage) { m_Usage = usage; }
        std::string GetUsage() { return m_Usage; }

        void ParseOptions(std::vector<std::string> &arguments);
        Options GetOptions() const { return m_Options; }

        void SetHeadless(bool headless) { m_Headless = headless; }
        bool IsHeadless() const { return m_Headless; }

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
        LayerStack m_LayerStack;

        float m_Fps{0.0f};
        float m_FrameTime{0.0f};
        uint32_t m_FrameCount{0};
        uint32_t m_LastFrameCount{0};
        Timer m_Timer{};

        std::unique_ptr<Instance> m_Instance{};
        std::unique_ptr<Device> m_Device{};
        std::unique_ptr<RenderPipeline> m_RenderPipeline{};
        std::vector<Scene *> m_Scenes{};
        /* std::unique_ptr<Scene> m_Scene{};
        std::unique_ptr<Gui> m_Gui{};
        Window *m_Window;
        Window *m_Window2; */

        std::unordered_map<const char *, bool> m_DeviceExtensions{};
        std::unordered_map<const char *, bool> m_InstanceExtensions{};
        std::vector<const char *> m_ValidationLayers{};
    };

    std::unique_ptr<Application> CreateApplication(Platform *platform);
}

extern std::unique_ptr<engine::Application> engine::CreateApplication(engine::Platform *platform);