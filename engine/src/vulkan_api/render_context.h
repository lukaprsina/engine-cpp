#pragma once

#include "vulkan_api/swapchain.h"
#include "vulkan_api/command_buffer.h"
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

        void Prepare(size_t thread_count = 1,
                     RenderTarget::CreateFunc create_render_target_function = RenderTarget::s_DefaultCreateFunction);

        CommandBuffer &Begin(CommandBuffer::ResetMode reset_mode = CommandBuffer::ResetMode::ResetPool);

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

    private:
        Device &m_Device;
        VkExtent2D m_SurfaceExtent;
        bool m_Prepared;
        std::unique_ptr<Swapchain> m_Swapchain;

        std::vector<std::unique_ptr<RenderFrame>> m_Frames;
    };
}