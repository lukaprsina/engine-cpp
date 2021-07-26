#pragma once

#include "vulkan_api/swapchain.h"

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

        void Prepare();

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
        VkExtent2D m_Extent;

        std::unique_ptr<Swapchain> m_Swapchain;
    };
}