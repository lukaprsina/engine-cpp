#include "core/application.h"

#include "core/log.h"
#include "events/application_event.h"
#include "vulkan_api/command_buffer.h"
#include "vulkan_api/device.h"
#include "vulkan_api/instance.h"
#include "platform/platform.h"
#include "vulkan_api/physical_device.h"
#include "vulkan_api/render_context.h"
#include "vulkan_api/render_target.h"
#include "vulkan_api/rendering/render_pipeline.h"
#include "renderer/shader.h"

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

namespace engine
{
    Application::Application(Platform *platform)
        : m_Platform(platform)
    {
        Log::Init();

        ENG_CORE_INFO("Logger initialized.");

        SetUsage(
            R"(Engine
    Usage:
        Engine [--headless]
        Engine --help
    Options:
        --help                    Show this screen.
        )"
#ifndef VK_USE_PLATFORM_DISPLAY_KHR
            R"(
        --width WIDTH             The width of the window [default: 1280].
        --height HEIGHT           The height of the window [default: 720].)"
#endif
            "\n");
    }

    Application::~Application()
    {
        if (m_Device)
            m_Device->WaitIdle();

        m_RenderContext.reset();
        m_Device.reset();

        if (m_Surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(m_Instance->GetHandle(), m_Surface, nullptr);

        m_Instance.reset();
    }

    bool Application::OnWindowClose(WindowCloseEvent & /*event*/)
    {
        m_Platform->Close();
        return true;
    }

    bool Application::Prepare()
    {
        AddInstanceExtension(m_Platform->GetSurfaceExtension());
        DebugUtilsSettings debug_utils_settings;

        m_Instance = std::make_unique<Instance>(m_Name,
                                                m_InstanceExtensions,
                                                m_ValidationLayers,
                                                debug_utils_settings,
                                                m_Headless,
                                                VK_API_VERSION_1_0);

        m_Surface = m_Platform->GetWindow().CreateSurface(*m_Instance);
        PhysicalDevice &gpu = m_Instance->GetBestGpu();

        if (gpu.GetFeatures().textureCompressionASTC_LDR)
            gpu.GetMutableRequestedFeatures().textureCompressionASTC_LDR = VK_TRUE;

        if (!IsHeadless() || m_Instance->IsExtensionEnabled(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME))
            AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        m_Device = std::make_unique<Device>(gpu, m_Surface, GetDeviceExtensions());

        std::vector<VkPresentModeKHR> present_mode_priority({VK_PRESENT_MODE_FIFO_KHR,
                                                             VK_PRESENT_MODE_MAILBOX_KHR});

        std::vector<VkSurfaceFormatKHR> surface_format_priority({{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                                 {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                                 {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                                 {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}});

        m_RenderContext = std::make_unique<RenderContext>(*m_Device,
                                                          m_Surface,
                                                          present_mode_priority,
                                                          surface_format_priority,
                                                          m_Platform->GetWindow().GetSettings().width,
                                                          m_Platform->GetWindow().GetSettings().height);

        m_RenderContext->Prepare();

        Shader vert_shader("base.vert");
        Shader frag_shader("base.frag");
        /* auto scene_subpass = std::make_unique<ForwardSubpass>(m_RenderContext,
                                                              std::move(vert_shader),
                                                              std::move(frag_shader),
                                                              *scene, *camera);

        auto render_pipeline = RenderPipeline();
        render_pipeline.AddSubpass(std::move(scene_subpass));

        m_RenderPipeline = std::make_unique<RenderPipeline>(std::move(render_pipeline)); */

        return true;
    }

    void Application::Step()
    {
        auto delta_time = static_cast<float>(m_Timer.Tick());

        if (m_FrameCount == 0)
            delta_time = 0.01667f;

        Update(delta_time);
    }

    void Application::Update(float delta_time)
    {
        UpdateScene(delta_time);
        auto &command_buffer = m_RenderContext->Begin();
        command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        Draw(command_buffer);
        command_buffer.End();
        m_RenderContext->Submit(command_buffer);
    }

    void Application::UpdateScene(float delta_time)
    {
    }

    void Application::Draw(CommandBuffer &command_buffer)
    {
        // auto &views = m_RenderContext->GetActiveFrame().GetRenderTarget().GetViews();
        // TODO memory barrier

        if (m_RenderPipeline)
            m_RenderPipeline->Draw(command_buffer,
                                   m_RenderContext->GetActiveFrame().GetRenderTarget());
    }

    void Application::Finish()
    {
        ENG_CORE_INFO("Closing Application.");
    }

    void Application::OnEvent(Event &event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
    }

    void Application::ParseOptions(std::vector<std::string> &arguments)
    {
        m_Options.ParseOptions(m_Usage, arguments);
    }
}