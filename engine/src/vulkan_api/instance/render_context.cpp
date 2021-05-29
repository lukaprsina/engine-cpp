#include "vulkan_api/instance/render_context.h"
#include "vulkan_api/instance/device.h"

namespace engine
{
    RenderContext::RenderContext(Device &device,
                                 VkSurfaceKHR surface,
                                 std::vector<VkPresentModeKHR> present_mode_priority,
                                 std::vector<VkSurfaceFormatKHR> surface_format_priority,
                                 uint32_t width,
                                 uint32_t height)
        : m_Device(device),
          m_PresentModePriority(present_mode_priority),
          m_SurfaceFormatPriority(surface_format_priority),
          m_Extent({width, height})
    {
    }

    RenderContext::~RenderContext()
    {
    }
}