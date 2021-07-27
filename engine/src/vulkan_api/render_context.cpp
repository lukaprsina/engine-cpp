#include "vulkan_api/render_context.h"

#include "vulkan_api/device.h"

namespace engine
{
    RenderContext::RenderContext(Device &device,
                                 VkSurfaceKHR surface,
                                 std::vector<VkPresentModeKHR> present_mode_priority,
                                 std::vector<VkSurfaceFormatKHR> surface_format_priority,
                                 uint32_t width,
                                 uint32_t height)
        : m_Device(device),
          m_Extent({width, height})
    {
        VkSurfaceCapabilitiesKHR surface_properties;
        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.GetGPU().GetHandle(),
                                                           surface,
                                                           &surface_properties));

        if (surface_properties.currentExtent.width == 0xFFFFFFFF)
            m_Swapchain = std::make_unique<Swapchain>(device, surface, present_mode_priority,
                                                      surface_format_priority, m_Extent);
        else
            m_Swapchain = std::make_unique<Swapchain>(device, surface, present_mode_priority,
                                                      surface_format_priority);
    }

    RenderContext::~RenderContext()
    {
    }

    void RenderContext::Prepare()
    {
        m_Device.WaitIdle();

        if (m_Swapchain)
        {
            m_Swapchain->Create();
        }
    }
}