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

        const std::vector<core::ImageView> &GetViews() const { return m_ImageViews; }
        const VkExtent2D &GetExtent() const { return m_Extent; }
        const std::vector<Attachment> &GetAttachments() const { return m_Attachments; }

        void SetInputAttachments(std::vector<uint32_t> &input) { m_InputAttachments = input; }
        const std::vector<uint32_t> &GetInputAttachments() const { return m_InputAttachments; }
        void SetOutputAttachments(std::vector<uint32_t> &output) { m_OutputAttachments = output; }
        const std::vector<uint32_t> &GetOutputAttachments() const { return m_OutputAttachments; }

    private:
        Device &m_Device;
        VkExtent2D m_Extent{};
        std::vector<core::Image> m_Images{};
        std::vector<core::ImageView> m_ImageViews{};
        std::vector<Attachment> m_Attachments{};

        /// By default there are no input attachments
        std::vector<uint32_t> m_InputAttachments = {};

        /// By default the output attachments is attachment 0
        std::vector<uint32_t> m_OutputAttachments = {0};
    };
}
