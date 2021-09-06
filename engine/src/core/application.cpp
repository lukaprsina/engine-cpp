#include "core/application.h"

#include "core/log.h"
#include "core/timer.h"
#include "events/application_event.h"
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

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

namespace engine
{
    Application::Application(Platform *platform)
        : m_Platform(platform)
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
        --width WIDTH             The width of the window [default: 1280].
        --height HEIGHT           The height of the window [default: 720].)"
#endif
            "\n");
    }

    Application::~Application()
    {
        if (m_Device)
            m_Device->WaitIdle();

        m_Scene.reset();

        m_RenderContext.reset();
        m_Device.reset();

        if (m_Surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(m_Instance->GetHandle(), m_Surface, nullptr);

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

        m_Surface = m_Platform->GetWindow().CreateSurface(*m_Instance);
        PhysicalDevice &gpu = m_Instance->GetBestGpu();

        if (gpu.GetFeatures().textureCompressionASTC_LDR)
            gpu.GetMutableRequestedFeatures().textureCompressionASTC_LDR = VK_TRUE;

        if (!IsHeadless() || m_Instance->IsExtensionEnabled(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME))
            AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        m_Device = std::make_unique<Device>(gpu, m_Surface, GetDeviceExtensions());

        std::vector<VkPresentModeKHR> present_mode_priority({VK_PRESENT_MODE_FIFO_KHR,
                                                             VK_PRESENT_MODE_IMMEDIATE_KHR,
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

        ShaderSource vert_shader("base.vert");
        ShaderSource frag_shader("base.frag");

        std::string scene;

        if (m_Options.Contains("<scene>"))
            scene = m_Options.GetString("<scene>");

        if (scene.empty())
            LoadScene("scenes/sponza/Sponza01.gltf");
        else
            LoadScene(scene);

        m_Scene->AddFreeCamera(m_RenderContext->GetSurfaceExtent());

        auto scene_subpass = std::make_unique<ForwardSubpass>(GetRenderContext(),
                                                              std::move(vert_shader),
                                                              std::move(frag_shader),
                                                              *m_Scene);

        m_RenderPipeline = std::make_unique<RenderPipeline>();
        m_RenderPipeline->AddSubpass(std::move(scene_subpass));

        return true;
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

            ENG_CORE_TRACE("FPS: {:.1f}", m_Fps);

            m_LastFrameCount = m_FrameCount;
            m_Timer.Lap();
        }

        m_FrameCount++;
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
        auto view = m_Scene->GetRegistry().view<sg::FreeCamera>();

        for (auto &entity : view)
        {
            auto &camera = view.get<sg::FreeCamera>(entity);
            camera.Update(delta_time);
        }
    }

    void Application::Draw(CommandBuffer &command_buffer)
    {
        auto &views = m_RenderContext->GetActiveFrame().GetRenderTarget().GetViews();

        {
            // Image 0 is the swapchain
            ImageMemoryBarrier memory_barrier{};
            memory_barrier.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
            memory_barrier.new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            memory_barrier.src_access_mask = 0;
            memory_barrier.dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            command_buffer.CreateImageMemoryBarrier(views.at(0), memory_barrier);

            // Skip 1 as it is handled later as a depth-stencil attachment
            for (size_t i = 2; i < views.size(); ++i)
            {
                command_buffer.CreateImageMemoryBarrier(views.at(i), memory_barrier);
            }
        }

        {
            ImageMemoryBarrier memory_barrier{};
            memory_barrier.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
            memory_barrier.new_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            memory_barrier.src_access_mask = 0;
            memory_barrier.dst_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

            command_buffer.CreateImageMemoryBarrier(views.at(1), memory_barrier);
        }

        SetViewportAndScissor(command_buffer,
                              m_RenderContext->GetActiveFrame().GetRenderTarget().GetExtent());

        if (m_RenderPipeline)
            m_RenderPipeline->Draw(command_buffer,
                                   m_RenderContext->GetActiveFrame().GetRenderTarget());

        command_buffer.EndRenderPass();

        {
            ImageMemoryBarrier memory_barrier{};
            memory_barrier.old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            memory_barrier.new_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            memory_barrier.src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

            command_buffer.CreateImageMemoryBarrier(views.at(0), memory_barrier);
        }
    }

    void Application::Finish()
    {
        auto execution_time = m_Timer.Stop();
        ENG_CORE_INFO("Closing Application. (Runtime: {:.1f})", execution_time);
    }

    void Application::OnEvent(Event &event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnResize));
    }

    bool Application::OnWindowClose(WindowCloseEvent & /*event*/)
    {
        m_Platform->Close();
        return true;
    }

    bool Application::OnResize(WindowResizeEvent &event)
    {
        auto view = m_Scene->GetRegistry().view<sg::FreeCamera>();
        for (auto &entity : view)
        {
            auto &free_camera = view.get<sg::FreeCamera>(entity);
            free_camera.Resize(event.GetWidth(), event.GetHeight());
        }

        return true;
    }

    void Application::ParseOptions(std::vector<std::string> &arguments)
    {
        m_Options.ParseOptions(m_Usage, arguments);
    }

    void Application::LoadScene(const std::string &path)
    {
        GLTFLoader loader(*m_Device);

        m_Scene = loader.ReadSceneFromFile(path);

        if (!m_Scene)
            throw std::runtime_error("Cannot load scene: " + path);
    }

    void Application::SetViewportAndScissor(CommandBuffer &command_buffer, const VkExtent2D &extent) const
    {
        VkViewport viewport{};
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        command_buffer.SetViewport(0, {viewport});

        VkRect2D scissor{};
        scissor.extent = extent;
        command_buffer.SetScissor(0, {scissor});
    }
}