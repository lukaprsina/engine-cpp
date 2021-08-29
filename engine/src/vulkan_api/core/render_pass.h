#pragma once

namespace engine
{
    class Attachment;
    class Device;
    struct SubpassInfo
    {
        std::vector<uint32_t> input_attachments;
        std::vector<uint32_t> output_attachments;
        std::vector<uint32_t> color_resolve_attachments;
        bool disable_depth_stencil_attachment;
        uint32_t depth_stencil_resolve_attachment;
        VkResolveModeFlagBits depth_stencil_resolve_mode;
    };
    class RenderPass
    {
    public:
        RenderPass(Device &device,
                   const std::vector<Attachment> &attachments,
                   const std::vector<LoadStoreInfo> &load_store_infos,
                   const std::vector<SubpassInfo> &subpasses);

        RenderPass(const RenderPass &) = delete;
        RenderPass(RenderPass &&other);
        ~RenderPass();
        RenderPass &operator=(const RenderPass &) = delete;
        RenderPass &operator=(RenderPass &&) = delete;

        const VkExtent2D GetRenderAreaGranularity() const;
        const uint32_t GetColorOutputCount(uint32_t subpass_index) const;
        VkRenderPass GetHandle() const { return m_Handle; };

    private:
        Device &m_Device;
        VkRenderPass m_Handle{VK_NULL_HANDLE};
        size_t m_SubpassCount{0};
        std::vector<uint32_t> m_ColorOutputCount{};

        template <typename T_SubpassDescription, typename T_AttachmentDescription, typename T_AttachmentReference, typename T_SubpassDependency, typename T_RenderPassCreateInfo>
        void CreateRenderpass(const std::vector<Attachment> &attachments, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<SubpassInfo> &subpasses);
    };
}
