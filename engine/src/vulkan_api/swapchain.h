#pragma once

#include <set>

namespace engine
{
    class Device;
    struct SwapchainProperties
    {
        VkSwapchainKHR old_swapchain;
        uint32_t image_count{3};
        VkExtent2D extent{};
        VkSurfaceFormatKHR surface_format{};
        uint32_t array_layers;
        VkImageUsageFlags image_usage;
        VkSurfaceTransformFlagBitsKHR pre_transform;
        VkCompositeAlphaFlagBitsKHR composite_alpha;
        VkPresentModeKHR present_mode;
    };

    class Swapchain
    {
    public:
        Swapchain(Device &device,
                  VkSurfaceKHR surface,
                  std::vector<VkPresentModeKHR> present_mode_priority,
                  std::vector<VkSurfaceFormatKHR> surface_format_priority,
                  const VkExtent2D &extent = {},
                  const uint32_t image_count = 3,
                  const VkSurfaceTransformFlagBitsKHR transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                  const std::set<VkImageUsageFlagBits> &image_usage_flags = {VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT});

        Swapchain(Swapchain &old_swapchain,
                  Device &device,
                  VkSurfaceKHR surface,
                  std::vector<VkPresentModeKHR> present_mode_priority,
                  std::vector<VkSurfaceFormatKHR> surface_format_priority,
                  const VkExtent2D &extent = {},
                  const uint32_t image_count = 3,
                  const VkSurfaceTransformFlagBitsKHR transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                  const std::set<VkImageUsageFlagBits> &image_usage_flags = {VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT});

        Swapchain(Swapchain &old_swapchain, const VkExtent2D &extent);

        ~Swapchain();

        VkSurfaceKHR GetSurface() const { return m_Surface; }

        VkSwapchainKHR GetHandle() const { return m_Handle; }

        const VkExtent2D &GetExtent() const { return m_Properties.extent; }

        const std::vector<VkImage> &GetImages() const { return m_Images; }

        VkFormat GetFormat() const { return m_Properties.surface_format.format; }

        VkImageUsageFlags GetUsage() const { return m_Properties.image_usage; }

        void SetPresentModePriority(const std::vector<VkPresentModeKHR> &new_present_mode_priority_list)
        {
            m_PresentModePriorityList = new_present_mode_priority_list;
        }

        void SetSurfaceFormatPriority(const std::vector<VkSurfaceFormatKHR> &new_surface_format_priority_list)
        {
            m_SurfaceFormatPriorityList = new_surface_format_priority_list;
        }

        void Create();

    private:
        Device &m_Device;
        VkSurfaceKHR m_Surface{VK_NULL_HANDLE};
        VkSwapchainKHR m_Handle{VK_NULL_HANDLE};
        std::vector<VkImage> m_Images;

        std::vector<VkPresentModeKHR> m_PresentModes;
        std::vector<VkSurfaceFormatKHR> m_SurfaceFormats;

        SwapchainProperties m_Properties;

        // A list of present modes in order of priority (vector[0] has high priority, vector[size-1] has low priority)
        std::vector<VkPresentModeKHR> m_PresentModePriorityList = {
            VK_PRESENT_MODE_FIFO_KHR,
            VK_PRESENT_MODE_MAILBOX_KHR};

        // A list of surface formats in order of priority (vector[0] has high priority, vector[size-1] has low priority)
        std::vector<VkSurfaceFormatKHR> m_SurfaceFormatPriorityList = {
            {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
            {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
            {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
            {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};

        std::set<VkImageUsageFlagBits> m_ImageUsageFlags;
    };
}
