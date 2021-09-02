#pragma once

#include "vulkan_api/swapchain.h"
#include "vulkan_api/command_buffer.h"
#include "vulkan_api/queue.h"
#include "vulkan_api/queue_family.h"
#include "vulkan_api/render_target.h"
#include "vulkan_api/render_frame.h"

namespace engine
{
    class Device;
    class RenderContext
    {
    public:
        RenderContext(Device &device,
                      VkSurfaceKHR surface,
                      std::vector<VkPresentModeKHR> present_mode_priority,
                      std::vector<VkSurfaceFormatKHR> surface_format_priority,
                      uint32_t width,
                      uint32_t height);
        ~RenderContext();

        void UpdateSwapchain(const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform);

        void Prepare(size_t thread_count = 1,
                     RenderTarget::CreateFunc create_render_target_function = RenderTarget::s_DefaultCreateFunction);

        CommandBuffer &Begin(CommandBuffer::ResetMode reset_mode = CommandBuffer::ResetMode::ResetPool);

        void Submit(CommandBuffer &command_buffer);
        void Submit(const std::vector<CommandBuffer *> &command_buffers);

        VkSemaphore Submit(const Queue &queue,
                           const std::vector<CommandBuffer *> &command_buffers,
                           VkSemaphore wait_semaphore,
                           VkPipelineStageFlags wait_pipeline_stage);

        void Submit(const Queue &queue,
                    const std::vector<CommandBuffer *> &command_buffers);

        void SetPresentModePriority(const std::vector<VkPresentModeKHR> &new_present_mode_priority_list)
        {
            if (m_Swapchain)
                m_Swapchain->SetPresentModePriority(new_present_mode_priority_list);
        }

        void SetSurfaceFormatPriority(const std::vector<VkSurfaceFormatKHR> &new_surface_format_priority_list)
        {
            if (m_Swapchain)
                m_Swapchain->SetSurfaceFormatPriority(new_surface_format_priority_list);
        }

        void HandleSurfaceChanges();
        RenderFrame &GetActiveFrame();

        VkSemaphore BeginFrame();
        void WaitFrame();
        void EndFrame(VkSemaphore semaphore);
        void Recreate();

        void ReleaseOwnedSemaphore(VkSemaphore semaphore);

        Device &GetDevice() { return m_Device; }
        VkExtent2D &GetSurfaceExtent() { return m_SurfaceExtent; }

    private:
        Device &m_Device;
        const QueueFamily &m_QueueFamily;
        VkExtent2D m_SurfaceExtent{};
        bool m_Prepared{false};
        std::unique_ptr<Swapchain> m_Swapchain{};
        RenderTarget::CreateFunc m_CreateRenderTargetFunction{RenderTarget::s_DefaultCreateFunction};
        VkSurfaceTransformFlagBitsKHR m_PreTransform{VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR};
        size_t m_ThreadCount{1};

        VkSemaphore m_AcquiredSemaphore{VK_NULL_HANDLE};
        uint32_t m_ActiveFrameIndex{0};
        bool m_FrameActive{false};

        std::vector<std::unique_ptr<RenderFrame>> m_Frames{};
    };
}