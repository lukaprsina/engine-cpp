#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>

#define VK_FLAGS_NONE 0

namespace engine
{
    bool IsDepthOnlyFormat(VkFormat format);
    bool IsDepthStencilFormat(VkFormat format);

    VkFormat GetSuitableDepthFormat(VkPhysicalDevice physical_device, bool depth_only = false,
                                    const std::vector<VkFormat> &depth_format_priority_list = {
                                        VK_FORMAT_D32_SFLOAT,
                                        VK_FORMAT_D24_UNORM_S8_UINT,
                                        VK_FORMAT_D16_UNORM});

}