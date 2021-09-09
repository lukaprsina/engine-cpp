#include "vulkan_api/render_context.h"

#include "vulkan_api/device.h"
#include "vulkan_api/core/image.h"

namespace engine
{
    RenderContext::RenderContext(Device &device,
                                 VkSurfaceKHR surface,
                                 std::vector<VkPresentModeKHR> present_mode_priority,
                                 std::vector<VkSurfaceFormatKHR> surface_format_priority,
                                 uint32_t width,
                                 uint32_t height)
        : m_Device(device),
          m_QueueFamily(device.GetSuitableGraphicsQueueFamily()),
          m_SurfaceExtent({width, height})
    {
        VkSurfaceCapabilitiesKHR surface_properties;
        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.GetGPU().GetHandle(),
                                                           surface,
                                                           &surface_properties));

        if (surface_properties.currentExtent.width == 0xFFFFFFFF)
            m_Swapchain = std::make_unique<Swapchain>(device, surface, present_mode_priority,
                                                      surface_format_priority, m_SurfaceExtent);
        else
            m_Swapchain = std::make_unique<Swapchain>(device, surface, present_mode_priority,
                                                      surface_format_priority);
    }

    RenderContext::~RenderContext()
    {
    }

    void RenderContext::UpdateSwapchain(const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform)
    {
        if (!m_Swapchain)
        {
            ENG_CORE_WARN("Can't update the swapchains extent and surface transform in headless mode, skipping.");
            return;
        }

        m_Device.GetResourceCache().ClearFramebuffers();

        auto width = extent.width;
        auto height = extent.height;
        // Pre-rotation: always use native orientation i.e. if rotated, use width and height of identity transform
        if (transform == VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR || transform == VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
            std::swap(width, height);

        m_Swapchain = std::make_unique<Swapchain>(*m_Swapchain, VkExtent2D{width, height}, transform);
        m_PreTransform = transform;

        Recreate();
    }

    void RenderContext::Prepare(size_t thread_count,
                                RenderTarget::CreateFunc create_render_target_function)
    {
        m_Device.WaitIdle();

        if (m_Swapchain)
        {
            m_Swapchain->Create();
            m_SurfaceExtent = m_Swapchain->GetExtent();

            VkExtent3D extent{m_SurfaceExtent.width, m_SurfaceExtent.height, 1};

            for (auto &image_handle : m_Swapchain->GetImages())
            {
                auto swapchain_image = core::Image(
                    m_Device, image_handle, extent,
                    m_Swapchain->GetFormat(),
                    m_Swapchain->GetUsage());

                auto render_target = create_render_target_function(std::move(swapchain_image));
                m_Frames.emplace_back(std::make_unique<RenderFrame>(m_Device, std::move(render_target),
                                                                    thread_count));
            }
        }

        m_CreateRenderTargetFunction = create_render_target_function;
        m_ThreadCount = thread_count;
        m_Prepared = true;
    }

    CommandBuffer &RenderContext::Begin(CommandBuffer::ResetMode reset_mode)
    {
        ENG_ASSERT(m_Prepared, "RenderContext not prepared for rendering, call prepare()");

        if (!m_FrameActive)
            BeginFrame();

        if (m_AcquiredSemaphore == VK_NULL_HANDLE)
            throw std::runtime_error("Couldn't begin frame!");

        const auto &queue_family = m_Device.GetQueueFamilyByFlags(VK_QUEUE_GRAPHICS_BIT);
        return GetActiveFrame().RequestCommandBuffer(queue_family, reset_mode);
    }

    void RenderContext::Submit(CommandBuffer &command_buffer)
    {
        Submit({&command_buffer});
    }

    void RenderContext::Submit(const std::vector<CommandBuffer *> &command_buffers)
    {
        ENG_ASSERT(m_FrameActive, "RenderContext is inactive, cannot submit command buffer. Please call begin()");

        VkSemaphore render_semaphore = VK_NULL_HANDLE;

        if (m_Swapchain)
        {
            ENG_ASSERT(m_AcquiredSemaphore, "We do not have acquired_semaphore, it was probably consumed?\n");
            render_semaphore = Submit(m_QueueFamily.GetQueues()[0], command_buffers, m_AcquiredSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        }
        else
        {
            Submit(m_QueueFamily.GetQueues()[0], command_buffers);
        }

        EndFrame(render_semaphore);
    }

    VkSemaphore RenderContext::Submit(const Queue &queue, const std::vector<CommandBuffer *> &command_buffers, VkSemaphore wait_semaphore, VkPipelineStageFlags wait_pipeline_stage)
    {
        std::vector<VkCommandBuffer> cmd_buf_handles(command_buffers.size(), VK_NULL_HANDLE);
        std::transform(command_buffers.begin(), command_buffers.end(), cmd_buf_handles.begin(), [](const CommandBuffer *cmd_buf)
                       { return cmd_buf->GetHandle(); });

        RenderFrame &frame = GetActiveFrame();

        VkSemaphore signal_semaphore = frame.RequestSemaphore();

        VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};

        submit_info.commandBufferCount = ToUint32_t(cmd_buf_handles.size());
        submit_info.pCommandBuffers = cmd_buf_handles.data();

        if (wait_semaphore != VK_NULL_HANDLE)
        {
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = &wait_semaphore;
            submit_info.pWaitDstStageMask = &wait_pipeline_stage;
        }

        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &signal_semaphore;

        VkFence fence = frame.RequestFence();

        queue.Submit({submit_info}, fence);

        return signal_semaphore;
    }

    void RenderContext::Submit(const Queue &queue, const std::vector<CommandBuffer *> &command_buffers)
    {
        std::vector<VkCommandBuffer> cmd_buf_handles(command_buffers.size(), VK_NULL_HANDLE);
        std::transform(command_buffers.begin(), command_buffers.end(), cmd_buf_handles.begin(), [](const CommandBuffer *cmd_buf)
                       { return cmd_buf->GetHandle(); });

        RenderFrame &frame = GetActiveFrame();

        VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};

        submit_info.commandBufferCount = ToUint32_t(cmd_buf_handles.size());
        submit_info.pCommandBuffers = cmd_buf_handles.data();

        VkFence fence = frame.RequestFence();

        queue.Submit({submit_info}, fence);
    }

    void RenderContext::BeginFrame()
    {
        if (m_Swapchain)
            HandleSurfaceChanges();

        ENG_ASSERT(!m_FrameActive, "Frame is still active, please call end_frame");
        auto &prev_frame = *m_Frames.at(m_ActiveFrameIndex);
        m_AcquiredSemaphore = prev_frame.RequestSemaphoreWithOwnership();

        if (m_Swapchain)
        {
            auto result = m_Swapchain->AcquireNextImage(m_ActiveFrameIndex, m_AcquiredSemaphore, VK_NULL_HANDLE);
            
            if (result == VK_SUBOPTIMAL_KHR || result == VK_NOT_READY || result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                bool swapchain_updated = HandleSurfaceChanges(result == VK_ERROR_OUT_OF_DATE_KHR ||
                    result == VK_NOT_READY);

                if (swapchain_updated)
                    result = m_Swapchain->AcquireNextImage(m_ActiveFrameIndex, m_AcquiredSemaphore, VK_NULL_HANDLE);
            }

            if (result != VK_SUCCESS)
            {
                // TODO: VK_NOT_READY 
                prev_frame.Reset();
                return;
            }
        }

        m_FrameActive = true;
        WaitFrame();
    }

    void RenderContext::WaitFrame()
    {
        RenderFrame &frame = GetActiveFrame();
        frame.Reset();
    }

    void RenderContext::EndFrame(VkSemaphore semaphore)
    {
        ENG_ASSERT(m_FrameActive, "Frame is not active, please call begin_frame");

        if (m_Swapchain)
        {
            VkSwapchainKHR vk_swapchain = m_Swapchain->GetHandle();

            VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};

            present_info.waitSemaphoreCount = 1;
            present_info.pWaitSemaphores = &semaphore;
            present_info.swapchainCount = 1;
            present_info.pSwapchains = &vk_swapchain;
            present_info.pImageIndices = &m_ActiveFrameIndex;

            VkResult result = m_QueueFamily.GetQueues()[0].Present(present_info);

            if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
                HandleSurfaceChanges();
        }

        // Frame is not active anymore
        if (m_AcquiredSemaphore)
        {
            ReleaseOwnedSemaphore(m_AcquiredSemaphore);
            m_AcquiredSemaphore = VK_NULL_HANDLE;
        }
        m_FrameActive = false;
    }

    RenderFrame &RenderContext::GetActiveFrame()
    {
        ENG_ASSERT(m_FrameActive, "Frame is not active, please call begin_frame");
        return *m_Frames.at(m_ActiveFrameIndex);
    }

    bool RenderContext::HandleSurfaceChanges(bool force_update)
    {
        if (!m_Swapchain)
        {
            ENG_CORE_WARN("Can't handle surface changes in headless mode, skipping.");
            return false;
        }
        VkSurfaceCapabilitiesKHR surface_properties;

        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Device.GetGPU().GetHandle(),
                                                           m_Swapchain->GetSurface(),
                                                           &surface_properties));

        if (surface_properties.currentExtent.width == 0xFFFFFFFF)
            return false;

        if (surface_properties.currentExtent.width != m_SurfaceExtent.width ||
            surface_properties.currentExtent.height != m_SurfaceExtent.height || force_update)
        {
            m_Device.WaitIdle();
            UpdateSwapchain(surface_properties.currentExtent, m_PreTransform);
            m_SurfaceExtent = surface_properties.currentExtent;

            return true;
        }

        return false;
    }

    void RenderContext::Recreate()
    {
        ENG_CORE_INFO("Recreating swapchain");

        VkExtent2D swapchain_extent = m_Swapchain->GetExtent();
        VkExtent3D extent{swapchain_extent.width, swapchain_extent.height, 1};

        auto frame_it = m_Frames.begin();

        for (auto &image_handle : m_Swapchain->GetImages())
        {
            core::Image swapchain_image{m_Device, image_handle,
                                        extent,
                                        m_Swapchain->GetFormat(),
                                        m_Swapchain->GetUsage()};

            auto render_target = m_CreateRenderTargetFunction(std::move(swapchain_image));

            if (frame_it != m_Frames.end())
            {
                (*frame_it)->UpdateRenderTarget(std::move(render_target));
            }
            else
            {
                // Create a new frame if the new swapchain has more images than current frames
                m_Frames.emplace_back(std::make_unique<RenderFrame>(m_Device, std::move(render_target), m_ThreadCount));
            }

            ++frame_it;
        }

        m_Device.GetResourceCache().ClearFramebuffers();
    }

    void RenderContext::ReleaseOwnedSemaphore(VkSemaphore semaphore)
    {
        RenderFrame &frame = GetActiveFrame();
        frame.ReleaseOwnedSemaphore(semaphore);
    }
}