#pragma once

#include "vulkan_api/core/image.h"
#include "vulkan_api/core/image_view.h"

namespace engine
{
    class Device;

    class Attachment
    {
    public:
        Attachment() = default;
        Attachment(VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage);

        VkFormat format = VK_FORMAT_UNDEFINED;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    };

    class RenderTarget
    {
    public:
        RenderTarget(std::vector<core::Image> &&images);
        ~RenderTarget();

        using CreateFunc = std::function<std::unique_ptr<RenderTarget>(core::Image &&)>;

        static const CreateFunc s_DefaultCreateFunction;

    private:
        Device &m_Device;
        VkExtent2D m_Extent{};
        std::vector<core::Image> m_Images{};
        std::vector<core::ImageView> m_ImageViews{};
        std::vector<Attachment> m_Attachments{};
    };
}
