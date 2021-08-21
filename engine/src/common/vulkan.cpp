#include "common/vulkan.h"

namespace engine
{
    bool IsDepthOnlyFormat(VkFormat format)
    {
        return format == VK_FORMAT_D16_UNORM ||
               format == VK_FORMAT_D32_SFLOAT;
    }

    bool IsDepthStencilFormat(VkFormat format)
    {
        return format == VK_FORMAT_D16_UNORM_S8_UINT ||
               format == VK_FORMAT_D24_UNORM_S8_UINT ||
               format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
               IsDepthOnlyFormat(format);
    }

    VkFormat GetSuitableDepthFormat(VkPhysicalDevice physical_device, bool depth_only,
                                    const std::vector<VkFormat> &depth_format_priority_list)
    {
        VkFormat depth_format{VK_FORMAT_UNDEFINED};

        for (auto &format : depth_format_priority_list)
        {
            if (depth_only && !IsDepthOnlyFormat(format))
            {
                continue;
            }

            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(physical_device, format, &properties);

            // Format must support depth stencil attachment for optimal tiling
            if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                depth_format = format;
                break;
            }
        }

        if (depth_format != VK_FORMAT_UNDEFINED)
        {
            ENG_CORE_INFO("Depth format selected: {}", ToString(depth_format));
            return depth_format;
        }

        throw std::runtime_error("No suitable depth format could be determined");
    }

    bool IsDynamicBufferDescriptorType(VkDescriptorType descriptor_type)
    {
        return descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ||
               descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    }

    bool IsBufferDescriptorType(VkDescriptorType descriptor_type)
    {
        return descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
               descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
               IsDynamicBufferDescriptorType(descriptor_type);
    }
}