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
        : m_Device(device), m_SurfaceExtent({width, height}),
          m_Prepared(false)
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

            m_Prepared = true;
        }
    }

    CommandBuffer &RenderContext::Begin(CommandBuffer::ResetMode reset_mode)
    {
        assert(m_Prepared);
    }
}