#pragma once

#include "vulkan_api/instance/swapchain.h"

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

    private:
        Device &m_Device;

        std::vector<VkPresentModeKHR> m_PresentModePriority;
        std::vector<VkSurfaceFormatKHR> m_SurfaceFormatPriority;
        VkExtent2D m_Extent;

        std::unique_ptr<Swapchain> m_Swapchain;
    };
}