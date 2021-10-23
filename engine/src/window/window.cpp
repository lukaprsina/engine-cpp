#include "window/window.h"

#include "platform/platform.h"
#include "vulkan_api/device.h"
#include "events/event.h"
#include "events/application_event.h"
#include "vulkan_api/render_context.h"
#include "vulkan_api/rendering/render_pipeline.h"
#include "vulkan_api/subpasses/forward_subpass.h"
#include "scene/scene.h"
#include "core/application.h"

namespace engine
{
    Window::Window(Platform &platform, WindowSettings &settings)
        : m_Platform(platform), m_Settings(settings), m_Input(this)
    {
        SetEventCallback(ENG_BIND_CALLBACK(Window::OnEvent));

        MouseMovedEvent event(this, static_cast<float>(1), static_cast<float>(2));
        m_Settings.EventCallback(event);
    }

    Window::~Window()
    {
        for (Layer *layer : m_Layers)
        {
            m_Platform.GetApp().GetLayerStack().PopLayer(layer);
        }

        m_RenderContext.reset();

        if (m_Surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(m_Platform.GetApp().GetInstance().GetHandle(), m_Surface, nullptr);
    }

    void Window::Draw()
    {
        for (Layer *layer : m_Layers)
        {
            if (!layer->IsInitialized())
                continue;

            auto &command_buffer = m_RenderContext->Begin();
            command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
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

            auto &render_target = m_RenderContext->GetActiveFrame().GetRenderTarget();
            SetViewportAndScissor(command_buffer,
                                  render_target.GetExtent());

            Render(command_buffer, render_target, layer);

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

            command_buffer.End();
            m_CommandBuffers.emplace_back(&command_buffer);
        }
        m_RenderContext->Submit(m_CommandBuffers);
        m_CommandBuffers.clear();
    }

    void Window::Render(CommandBuffer &command_buffer, RenderTarget &render_target, Layer *layer)
    {
        Scene *scene = layer->GetScene();
        layer->GetRenderPipeline()->Draw(*m_RenderContext, *layer, command_buffer, render_target);
    }

    void Window::SetViewportAndScissor(CommandBuffer &command_buffer, const VkExtent2D &extent) const
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

    void Window::OnEvent(Event &event)
    {
        EventDispatcher dispatcher(event);

        for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it)
        {
            if (event.handled)
                break;
            (*it)->OnEvent(event);
        }

        m_Platform.GetApp().OnEvent(event);
    }

    void Window::AddScene(Scene *scene)
    {
        m_Scenes.emplace_back(scene);
    }

    void Window::AddLayer(Layer *layer)
    {
        m_Layers.emplace_back(layer);
    }

    void Window::SetSettings(WindowSettings &settings)
    {
        m_Settings = settings;
        m_Dirty = true;
    }

    RenderContext &Window::CreateRenderContext(Device &device,
                                               std::vector<VkPresentModeKHR> &present_mode_priority,
                                               std::vector<VkSurfaceFormatKHR> &surface_format_priority)
    {
        m_RenderContext = std::make_unique<RenderContext>(device,
                                                          m_Surface,
                                                          present_mode_priority,
                                                          surface_format_priority,
                                                          m_Settings.width,
                                                          m_Settings.height);

        return *m_RenderContext;
    }

    RenderContext &Window::GetRenderContext()
    {
        ENG_ASSERT(m_RenderContext, "Render context is not valid");
        return *m_RenderContext;
    }

    void Window::DeleteRenderContext()
    {
        m_RenderContext.reset();
    }
}