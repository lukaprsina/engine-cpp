#include "core/application.h"

#include "window/window.h"
#include "core/timer.h"
#include "core/gui.h"
#include "core/layer_stack.h"
#include "events/application_event.h"
#include "events/key_event.h"
#include "window/input.h"
#include "vulkan_api/command_buffer.h"
#include "vulkan_api/device.h"
#include "vulkan_api/instance.h"
#include "platform/platform.h"
#include "vulkan_api/physical_device.h"
#include "vulkan_api/render_context.h"
#include "vulkan_api/render_target.h"
#include "vulkan_api/rendering/render_pipeline.h"
#include "vulkan_api/subpasses/forward_subpass.h"
#include "renderer/shader.h"
#include "scene/components/perspective_camera.h"
#include "scene/scene.h"
#include "scene/scripts/free_camera.h"
#include "scene/gltf_loader.h"

#include <set>

namespace engine
{
    Application::Application(Platform *platform)
        : m_Platform(platform), m_LayerStack(*this)
    {
        Log::Init();
        // Log::GetCoreLogger()->set_level(spdlog::level::warn);

        ENG_CORE_INFO("Logger initialized.");

        SetUsage(
            R"(Engine
    Usage:
        engine [--headless]
        engine <scene> [--headless]
        engine --help
    Options:
        --help                    Show this screen.
        )"
#ifndef VK_USE_PLATFORM_DISPLAY_KHR
            R"(
        --width WIDTH             The width of the window.
        --height HEIGHT           The height of the window.)"
#endif
            "\n");
    }

    Application::~Application()
    {
        if (m_Device)
            m_Device->WaitIdle();

        for (auto &layer : m_LayerStack.GetLayers())
        {
            layer.second->OnDetach();
            layer.second.reset();
        }

        m_Device.reset();

        m_Instance.reset();
    }

    bool Application::Prepare()
    {
        m_Timer.Start();
        AddInstanceExtension(m_Platform->GetSurfaceExtension());
        DebugUtilsSettings debug_utils_settings;

        m_Instance = std::make_unique<Instance>(m_Name,
                                                m_InstanceExtensions,
                                                m_ValidationLayers,
                                                debug_utils_settings,
                                                m_Headless,
                                                VK_API_VERSION_1_0);

        PhysicalDevice &gpu = m_Instance->GetBestGpu();

        if (gpu.GetFeatures().textureCompressionASTC_LDR)
            gpu.GetMutableRequestedFeatures()
                .textureCompressionASTC_LDR = VK_TRUE;

        if (!IsHeadless() || m_Instance->IsExtensionEnabled(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME))
            AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        m_Device = std::make_unique<Device>(gpu, *m_Platform, GetDeviceExtensions());

        return true;

        /* std::vector<VkPresentModeKHR> present_mode_priority({VK_PRESENT_MODE_IMMEDIATE_KHR,
                                                             VK_PRESENT_MODE_FIFO_KHR,
                                                             VK_PRESENT_MODE_MAILBOX_KHR});

        std::vector<VkSurfaceFormatKHR> surface_format_priority({{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                                 {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                                 {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                                 {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}});


        std::string scene;

        if (m_Options.Contains("<scene>"))
            scene = m_Options.GetString("<scene>");

        if (scene.empty())
            LoadScene("scenes/sponza/Sponza01.gltf");
        else
            LoadScene(scene);

        

        m_Gui = std::make_unique<Gui>(*this, m_Window);

        m_LayerStack.PushLayer(m_Gui.get()); */
    }

    void Application::Step()
    {
        auto delta_time = static_cast<float>(m_Timer.Tick());

        if (m_FrameCount == 0)
            delta_time = 0.01667f;

        Update(delta_time);

        auto elapsed_time = static_cast<float>(m_Timer.Elapsed<Timer::Seconds>());

        if (elapsed_time > 0.5f)
        {
            m_Fps = (m_FrameCount - m_LastFrameCount) / elapsed_time;
            m_FrameTime = delta_time * 1000.0f;

            // ENG_CORE_TRACE("FPS: {:.1f}", m_Fps);

            m_LastFrameCount = m_FrameCount;
            m_Timer.Lap();
        }

        m_FrameCount++;
    }

    void Application::Update(float delta_time)
    {
        std::set<Scene *> scenes;
        for (auto &layer : m_LayerStack.GetLayers())
        {
            if (!layer.second->IsInitialized())
            {
                layer.second->OnAttach();
                layer.second->m_Initialized = true;
            }

            layer.second->OnUpdate(delta_time);
            scenes.emplace(layer.second->GetScene());
        }

        for (Scene *scene : scenes)
        {
            if (scene)
                scene->Update(delta_time);
        }
    }

    void Application::Finish()
    {
        auto execution_time = m_Timer.Stop();
        ENG_CORE_INFO("Closing Application. (Runtime: {:.1f})", execution_time);
    }

    Scene *Application::LoadScene()
    {
        auto scene = std::make_unique<Scene>();
        m_Scenes.emplace_back(std::move(scene));
        return m_Scenes.back().get();
    }

    Scene *Application::LoadScene(std::string name)
    {
        GLTFLoader loader(*m_Device);
        auto scene = loader.ReadSceneFromFile(name);
        m_Scenes.emplace_back(std::move(scene));
        return m_Scenes.back().get();
    }

    void Application::OnEvent(Event &event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>(ENG_BIND_CALLBACK(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(ENG_BIND_CALLBACK(Application::OnResize));
        dispatcher.Dispatch<KeyPressedEvent>(ENG_BIND_CALLBACK(Application::OnKeyPressed));
    }

    bool Application::OnWindowClose(WindowCloseEvent & /*event*/)
    {
        return false;
    }

    bool Application::OnResize(WindowResizeEvent &event)
    {
        /* auto view = m_Scene->GetRegistry().view<sg::FreeCamera>();
        for (auto &entity : view)
        {
            auto &free_camera = view.get<sg::FreeCamera>(entity);
            free_camera.Resize(event.GetWidth(), event.GetHeight());
        } */

        return false;
    }

    bool Application::OnKeyPressed(KeyPressedEvent &event)
    {
        /* bool alt_enter = m_Window->GetInput().IsKeyPressed(Key::LeftAlt) && m_Window->GetInput().IsKeyPressed(Key::Enter);

        if (event.GetKeyCode() == Key::F11 || alt_enter)
        {
            auto window_settings = m_Window->GetSettings();
            window_settings.fullscreen = !window_settings.fullscreen;
            m_Window->SetSettings(window_settings);
        } */

        return false;
    }

    void Application::ParseOptions(std::vector<std::string> &arguments)
    {
        m_Options.ParseOptions(m_Usage, arguments);
    }
}