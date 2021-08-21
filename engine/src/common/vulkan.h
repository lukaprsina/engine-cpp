#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>

#define VK_FLAGS_NONE 0

namespace engine
{
    struct ImageMemoryBarrier
    {
        VkPipelineStageFlags src_stage_mask{VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
        VkPipelineStageFlags dst_stage_mask{VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
        VkAccessFlags src_access_mask{0};
        VkAccessFlags dst_access_mask{0};
        VkImageLayout old_layout{VK_IMAGE_LAYOUT_UNDEFINED};
        VkImageLayout new_layout{VK_IMAGE_LAYOUT_UNDEFINED};
        uint32_t old_queue_family{VK_QUEUE_FAMILY_IGNORED};
        uint32_t new_queue_family{VK_QUEUE_FAMILY_IGNORED};
    };

    struct LoadStoreInfo
    {
        VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
        VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE;
    };

    bool IsDepthOnlyFormat(VkFormat format);
    bool IsDepthStencilFormat(VkFormat format);

    VkFormat GetSuitableDepthFormat(VkPhysicalDevice physical_device, bool depth_only = false,
                                    const std::vector<VkFormat> &depth_format_priority_list = {
                                        VK_FORMAT_D32_SFLOAT,
                                        VK_FORMAT_D24_UNORM_S8_UINT,
                                        VK_FORMAT_D16_UNORM});

    bool IsDynamicBufferDescriptorType(VkDescriptorType descriptor_type);
    bool IsBufferDescriptorType(VkDescriptorType descriptor_type);

}