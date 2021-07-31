#include "vulkan_api/swapchain.h"
#include "vulkan_api/device.h"

namespace engine
{
    namespace
    {
        inline uint32_t ChooseImageCount(
            uint32_t request_image_count,
            uint32_t min_image_count,
            uint32_t max_image_count)
        {
            if (max_image_count != 0)
            {
                request_image_count = std::min(request_image_count, max_image_count);
            }

            request_image_count = std::max(request_image_count, min_image_count);

            return request_image_count;
        }

        inline uint32_t ChooseImageArrayLayers(
            uint32_t request_image_array_layers,
            uint32_t max_image_array_layers)
        {
            request_image_array_layers = std::min(request_image_array_layers, max_image_array_layers);
            request_image_array_layers = std::max(request_image_array_layers, 1U);

            return request_image_array_layers;
        }

        inline VkExtent2D ChooseExtent(
            VkExtent2D request_extent,
            const VkExtent2D &min_image_extent,
            const VkExtent2D &max_image_extent,
            const VkExtent2D &current_extent)
        {
            if (current_extent.width == 0xFFFFFFFF)
            {
                return request_extent;
            }

            if (request_extent.width < 1 || request_extent.height < 1)
            {
                ENG_CORE_WARN("(Swapchain) Image extent ({}, {}) not supported. Selecting ({}, {}).", request_extent.width, request_extent.height, current_extent.width, current_extent.height);
                return current_extent;
            }

            request_extent.width = std::max(request_extent.width, min_image_extent.width);
            request_extent.width = std::min(request_extent.width, max_image_extent.width);

            request_extent.height = std::max(request_extent.height, min_image_extent.height);
            request_extent.height = std::min(request_extent.height, max_image_extent.height);

            return request_extent;
        }

        inline VkPresentModeKHR ChoosePresentMode(
            const std::vector<VkPresentModeKHR> &available_present_modes,
            const std::vector<VkPresentModeKHR> &present_mode_priority_list)

        {
            VkPresentModeKHR chosen_present_mode = VK_PRESENT_MODE_FIFO_KHR;

            for (auto &present_mode : present_mode_priority_list)
            {
                if (std::find(available_present_modes.begin(), available_present_modes.end(), present_mode) != available_present_modes.end())
                {
                    chosen_present_mode = present_mode;
                    ENG_CORE_INFO("(Swapchain) Present mode selected: {}", ToString(chosen_present_mode));
                    break;
                }
                else
                {
                    ENG_CORE_WARN("(Swapchain) No present modes from the priority list found. Selecting ({}).", ToString(chosen_present_mode));
                }
            }

            return chosen_present_mode;
        }

        inline VkSurfaceFormatKHR ChooseSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &available_surface_formats,
            const std::vector<VkSurfaceFormatKHR> &surface_format_priority_list)
        {
            auto surface_format_it = available_surface_formats.begin();

            for (auto &surface_format : surface_format_priority_list)
            {
                surface_format_it = std::find_if(
                    available_surface_formats.begin(),
                    available_surface_formats.end(),
                    [&surface_format](const VkSurfaceFormatKHR &surface)
                    {
                        if (surface.format == surface_format.format &&
                            surface.colorSpace == surface_format.colorSpace)
                        {
                            return true;
                        }

                        return false;
                    });

                if (surface_format_it != available_surface_formats.end())
                {
                    ENG_CORE_INFO("(Swapchain) Surface format selected: {}", ToString(*surface_format_it));
                    return *surface_format_it;
                }
            }

            // If nothing found, default the first supported surface format
            surface_format_it = available_surface_formats.begin();
            ENG_CORE_WARN("(Swapchain) No surface formats from the priority list found. Selecting ({}).", ToString(*surface_format_it));

            return *surface_format_it;
        }

        inline VkSurfaceTransformFlagBitsKHR ChooseTransform(
            VkSurfaceTransformFlagBitsKHR request_transform,
            VkSurfaceTransformFlagsKHR supported_transform,
            VkSurfaceTransformFlagBitsKHR current_transform)
        {
            if (request_transform & supported_transform)
            {
                return request_transform;
            }

            ENG_CORE_WARN("(Swapchain) Surface transform '{}' not supported. Selecting '{}'.", ToString(request_transform), ToString(current_transform));

            return current_transform;
        }

        inline VkCompositeAlphaFlagBitsKHR ChooseCompositeAlpha(VkCompositeAlphaFlagBitsKHR request_composite_alpha, VkCompositeAlphaFlagsKHR supported_composite_alpha)
        {
            if (request_composite_alpha & supported_composite_alpha)
            {
                return request_composite_alpha;
            }

            static const std::vector<VkCompositeAlphaFlagBitsKHR> composite_alpha_flags = {
                VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
                VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR};

            for (VkCompositeAlphaFlagBitsKHR composite_alpha : composite_alpha_flags)
            {
                if (composite_alpha & supported_composite_alpha)
                {
                    ENG_CORE_WARN("(Swapchain) Composite alpha '{}' not supported. Selecting '{}.", ToString(request_composite_alpha), ToString(composite_alpha));
                    return composite_alpha;
                }
            }

            throw std::runtime_error("No compatible composite alpha found.");
        }

        inline bool ValidateFormatFeature(VkImageUsageFlagBits image_usage, VkFormatFeatureFlags supported_features)
        {
            switch (image_usage)
            {
            case VK_IMAGE_USAGE_STORAGE_BIT:
                return VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT & supported_features;
            default:
                return true;
            }
        }

        inline std::set<VkImageUsageFlagBits> ChooseImageUsage(const std::set<VkImageUsageFlagBits> &requested_image_usage_flags, VkImageUsageFlags supported_image_usage, VkFormatFeatureFlags supported_features)
        {
            std::set<VkImageUsageFlagBits> validated_image_usage_flags;
            for (auto flag : requested_image_usage_flags)
            {
                if ((flag & supported_image_usage) && ValidateFormatFeature(flag, supported_features))
                {
                    validated_image_usage_flags.insert(flag);
                }
                else
                {
                    ENG_CORE_WARN("(Swapchain) Image usage ({}) requested but not supported.", ToString(flag));
                }
            }

            if (validated_image_usage_flags.empty())
            {
                // Pick the first format from list of defaults, if supported
                static const std::vector<VkImageUsageFlagBits> image_usage_flags = {
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    VK_IMAGE_USAGE_STORAGE_BIT,
                    VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT};

                for (VkImageUsageFlagBits image_usage : image_usage_flags)
                {
                    if ((image_usage & supported_image_usage) && ValidateFormatFeature(image_usage, supported_features))
                    {
                        validated_image_usage_flags.insert(image_usage);
                        break;
                    }
                }
            }

            if (!validated_image_usage_flags.empty())
            {
                // Log image usage flags used
                std::string usage_list;
                for (VkImageUsageFlagBits image_usage : validated_image_usage_flags)
                {
                    usage_list += ToString(image_usage) + " ";
                }
                ENG_CORE_INFO("(Swapchain) Image usage flags: {}", usage_list);
            }
            else
            {
                throw std::runtime_error("No compatible image usage found.");
            }

            return validated_image_usage_flags;
        }

        inline VkImageUsageFlags CompositeImageFlags(std::set<VkImageUsageFlagBits> &image_usage_flags)
        {
            VkImageUsageFlags image_usage{};
            for (auto flag : image_usage_flags)
            {
                image_usage |= flag;
            }
            return image_usage;
        }

    }

    Swapchain::Swapchain(Device &device,
                         VkSurfaceKHR surface,
                         std::vector<VkPresentModeKHR> present_mode_priority,
                         std::vector<VkSurfaceFormatKHR> surface_format_priority,
                         const VkExtent2D &extent,
                         const uint32_t image_count,
                         const VkSurfaceTransformFlagBitsKHR transform,
                         const std::set<VkImageUsageFlagBits> &image_usage_flags)
        : Swapchain(*this, device, surface, present_mode_priority, surface_format_priority,
                    extent, image_count, transform, image_usage_flags)
    {
    }

    Swapchain::Swapchain(Swapchain &old_swapchain,
                         Device &device,
                         VkSurfaceKHR surface,
                         std::vector<VkPresentModeKHR> present_mode_priority,
                         std::vector<VkSurfaceFormatKHR> surface_format_priority,
                         const VkExtent2D &extent,
                         const uint32_t image_count,
                         const VkSurfaceTransformFlagBitsKHR transform,
                         const std::set<VkImageUsageFlagBits> &image_usage_flags)
        : m_Device(device),
          m_Surface(surface)
    {
        m_PresentModePriorityList = present_mode_priority;
        m_SurfaceFormatPriorityList = surface_format_priority;

        VkSurfaceCapabilitiesKHR surface_capabilities{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Device.GetGPU().GetHandle(),
                                                  surface, &surface_capabilities);

        uint32_t surface_format_count = 0;

        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device.GetGPU().GetHandle(),
                                                      surface, &surface_format_count, nullptr));
        m_SurfaceFormats.resize(surface_format_count);
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device.GetGPU().GetHandle(),
                                                      surface, &surface_format_count, m_SurfaceFormats.data()));

        ENG_CORE_INFO("Surface supports the following surface formats:");
        for (auto &surface_format : m_SurfaceFormats)
        {
            ENG_CORE_INFO("  \t{}", ToString(surface_format));
        }

        uint32_t present_mode_count = 0;

        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Device.GetGPU().GetHandle(),
                                                           surface, &present_mode_count, nullptr));
        m_PresentModes.resize(present_mode_count);
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Device.GetGPU().GetHandle(),
                                                           surface, &present_mode_count, m_PresentModes.data()));

        ENG_CORE_INFO("Surface supports the following present modes:");
        for (auto &present_mode : m_PresentModes)
        {
            ENG_CORE_INFO("  \t{}", ToString(present_mode));
        }

        m_Properties.old_swapchain = old_swapchain.GetHandle();

        m_Properties.image_count = ChooseImageCount(image_count, surface_capabilities.minImageCount, surface_capabilities.maxImageCount);
        m_Properties.extent = ChooseExtent(extent, surface_capabilities.minImageExtent, surface_capabilities.maxImageExtent, surface_capabilities.currentExtent);
        m_Properties.array_layers = ChooseImageArrayLayers(1, surface_capabilities.maxImageArrayLayers);
        m_Properties.surface_format = ChooseSurfaceFormat(m_SurfaceFormats, m_SurfaceFormatPriorityList);

        VkFormatProperties format_properties;
        vkGetPhysicalDeviceFormatProperties(m_Device.GetGPU().GetHandle(), m_Properties.surface_format.format, &format_properties);
        m_ImageUsageFlags = ChooseImageUsage(image_usage_flags, surface_capabilities.supportedUsageFlags, format_properties.optimalTilingFeatures);
        m_Properties.image_usage = CompositeImageFlags(m_ImageUsageFlags);

        m_Properties.pre_transform = ChooseTransform(transform, surface_capabilities.supportedTransforms, surface_capabilities.currentTransform);
        m_Properties.composite_alpha = ChooseCompositeAlpha(VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, surface_capabilities.supportedCompositeAlpha);
    }

    Swapchain::Swapchain(Swapchain &old_swapchain, const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform)
        : Swapchain(old_swapchain, old_swapchain.m_Device,
                    old_swapchain.m_Surface,
                    old_swapchain.m_PresentModePriorityList,
                    old_swapchain.m_SurfaceFormatPriorityList,
                    extent,
                    old_swapchain.m_Properties.image_count,
                    transform,
                    old_swapchain.m_ImageUsageFlags)
    {
        m_PresentModePriorityList = old_swapchain.m_PresentModePriorityList;
        m_SurfaceFormatPriorityList = old_swapchain.m_SurfaceFormatPriorityList;
        Create();
    }

    Swapchain::~Swapchain()
    {
        if (m_Handle != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(m_Device.GetHandle(), m_Handle, nullptr);
        }
    }

    void Swapchain::Create()
    {
        m_Properties.present_mode = ChoosePresentMode(m_PresentModes, m_PresentModePriorityList);
        m_Properties.surface_format = ChooseSurfaceFormat(m_SurfaceFormats, m_SurfaceFormatPriorityList);

        VkSwapchainCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.minImageCount = m_Properties.image_count;
        create_info.imageExtent = m_Properties.extent;
        create_info.presentMode = m_Properties.present_mode;
        create_info.imageFormat = m_Properties.surface_format.format;
        create_info.imageColorSpace = m_Properties.surface_format.colorSpace;
        create_info.imageArrayLayers = m_Properties.array_layers;
        create_info.imageUsage = m_Properties.image_usage;
        create_info.preTransform = m_Properties.pre_transform;
        create_info.compositeAlpha = m_Properties.composite_alpha;
        create_info.oldSwapchain = m_Properties.old_swapchain;
        create_info.surface = m_Surface;

        VkResult result = vkCreateSwapchainKHR(m_Device.GetHandle(), &create_info, nullptr, &m_Handle);

        if (result != VK_SUCCESS)
        {
            throw VulkanException{result, "Cannot create Swapchain"};
        }

        uint32_t image_available;
        VK_CHECK(vkGetSwapchainImagesKHR(m_Device.GetHandle(), m_Handle, &image_available, nullptr));

        m_Images.resize(image_available);

        VK_CHECK(vkGetSwapchainImagesKHR(m_Device.GetHandle(), m_Handle, &image_available, m_Images.data()));
    }

    VkResult Swapchain::AcquireNextImage(uint32_t &image_index, VkSemaphore image_acquired_semaphore, VkFence fence)
    {
        return vkAcquireNextImageKHR(m_Device.GetHandle(), m_Handle,
                                     std::numeric_limits<uint64_t>::max(),
                                     image_acquired_semaphore, fence,
                                     &image_index);
    }
}
