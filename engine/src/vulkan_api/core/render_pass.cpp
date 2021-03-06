#include "vulkan_api/core/render_pass.h"

#include "vulkan_api/device.h"
#include "vulkan_api/render_target.h"

namespace engine
{
    namespace
    {
        inline void SetStructureType(VkAttachmentDescription &attachment)
        {
            // VkAttachmentDescription has no sType field
        }

        inline void SetStructureType(VkAttachmentDescription2KHR &attachment)
        {
            attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
        }

        inline void SetStructureType(VkAttachmentReference &reference)
        {
            // VkAttachmentReference has no sType field
        }

        inline void SetStructureType(VkAttachmentReference2KHR &reference)
        {
            reference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
        }

        inline void SetStructureType(VkRenderPassCreateInfo &create_info)
        {
            create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        }

        inline void SetStructureType(VkRenderPassCreateInfo2KHR &create_info)
        {
            create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR;
        }

        inline void SetStructureType(VkSubpassDescription &description)
        {
            // VkSubpassDescription has no sType field
        }

        inline void SetStructureType(VkSubpassDescription2KHR &description)
        {
            description.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR;
        }

        inline void SetPointerNext(VkSubpassDescription &subpass_description, VkSubpassDescriptionDepthStencilResolveKHR &depth_resolve, VkAttachmentReference &depth_resolve_attachment)
        {
            // VkSubpassDescription cannot have pNext point to a VkSubpassDescriptionDepthStencilResolveKHR containing a VkAttachmentReference
        }

        inline void SetPointerNext(VkSubpassDescription2KHR &subpass_description, VkSubpassDescriptionDepthStencilResolveKHR &depth_resolve, VkAttachmentReference2KHR &depth_resolve_attachment)
        {
            depth_resolve.pDepthStencilResolveAttachment = &depth_resolve_attachment;
            subpass_description.pNext = &depth_resolve;
        }

        inline const VkAttachmentReference2KHR *GetDepthResolveReference(const VkSubpassDescription &subpass_description)
        {
            // VkSubpassDescription cannot have pNext point to a VkSubpassDescriptionDepthStencilResolveKHR containing a VkAttachmentReference2KHR
            return nullptr;
        }

        inline const VkAttachmentReference2KHR *GetDepthResolveReference(const VkSubpassDescription2KHR &subpass_description)
        {
            auto description_depth_resolve = static_cast<const VkSubpassDescriptionDepthStencilResolveKHR *>(subpass_description.pNext);

            const VkAttachmentReference2KHR *depth_resolve_attachment = nullptr;
            if (description_depth_resolve)
            {
                depth_resolve_attachment = description_depth_resolve->pDepthStencilResolveAttachment;
            }

            return depth_resolve_attachment;
        }

        inline VkResult CreateVkRenderpass(VkDevice device, VkRenderPassCreateInfo &create_info, VkRenderPass *handle)
        {
            return vkCreateRenderPass(device, &create_info, nullptr, handle);
        }

        inline VkResult CreateVkRenderpass(VkDevice device, VkRenderPassCreateInfo2KHR &create_info, VkRenderPass *handle)
        {
            return vkCreateRenderPass2KHR(device, &create_info, nullptr, handle);
        }
    }

    template <typename T>
    std::vector<T> GetAttachmentDescriptions(const std::vector<Attachment> &attachments, const std::vector<LoadStoreInfo> &load_store_infos)
    {
        std::vector<T> attachment_descriptions;

        for (size_t i = 0U; i < attachments.size(); ++i)
        {
            T attachment{};
            SetStructureType(attachment);

            attachment.format = attachments[i].format;
            attachment.samples = attachments[i].samples;
            attachment.initialLayout = attachments[i].initial_layout;
            attachment.finalLayout = IsDepthStencilFormat(attachment.format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            if (i < load_store_infos.size())
            {
                attachment.loadOp = load_store_infos[i].load_op;
                attachment.storeOp = load_store_infos[i].store_op;
                attachment.stencilLoadOp = load_store_infos[i].load_op;
                attachment.stencilStoreOp = load_store_infos[i].store_op;
            }

            attachment_descriptions.push_back(std::move(attachment));
        }

        return attachment_descriptions;
    }

    template <typename T_SubpassDescription, typename T_AttachmentDescription, typename T_AttachmentReference>
    void SetAttachmentLayouts(std::vector<T_SubpassDescription> &subpass_descriptions, std::vector<T_AttachmentDescription> &attachment_descriptions)
    {
        // Make the initial layout same as in the first subpass using that attachment
        for (auto &subpass : subpass_descriptions)
        {
            for (size_t k = 0U; k < subpass.colorAttachmentCount; ++k)
            {
                auto &reference = subpass.pColorAttachments[k];
                // Set it only if not defined yet
                if (attachment_descriptions[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    attachment_descriptions[reference.attachment].initialLayout = reference.layout;
                }
            }

            for (size_t k = 0U; k < subpass.inputAttachmentCount; ++k)
            {
                auto &reference = subpass.pInputAttachments[k];
                // Set it only if not defined yet
                if (attachment_descriptions[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    attachment_descriptions[reference.attachment].initialLayout = reference.layout;
                }
            }

            if (subpass.pDepthStencilAttachment)
            {
                auto &reference = *subpass.pDepthStencilAttachment;
                // Set it only if not defined yet
                if (attachment_descriptions[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    attachment_descriptions[reference.attachment].initialLayout = reference.layout;
                }
            }

            if (subpass.pResolveAttachments)
            {
                for (size_t k = 0U; k < subpass.colorAttachmentCount; ++k)
                {
                    auto &reference = subpass.pResolveAttachments[k];
                    // Set it only if not defined yet
                    if (attachment_descriptions[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                    {
                        attachment_descriptions[reference.attachment].initialLayout = reference.layout;
                    }
                }
            }

            if (const auto depth_resolve = GetDepthResolveReference(subpass))
            {
                // Set it only if not defined yet
                if (attachment_descriptions[depth_resolve->attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    attachment_descriptions[depth_resolve->attachment].initialLayout = depth_resolve->layout;
                }
            }
        }

        // Make the final layout same as the last subpass layout
        {
            auto &subpass = subpass_descriptions.back();

            for (size_t k = 0U; k < subpass.colorAttachmentCount; ++k)
            {
                const auto &reference = subpass.pColorAttachments[k];

                attachment_descriptions[reference.attachment].finalLayout = reference.layout;
            }

            for (size_t k = 0U; k < subpass.inputAttachmentCount; ++k)
            {
                const auto &reference = subpass.pInputAttachments[k];

                attachment_descriptions[reference.attachment].finalLayout = reference.layout;

                // Do not use depth attachment if used as input
                if (IsDepthStencilFormat(attachment_descriptions[reference.attachment].format))
                {
                    subpass.pDepthStencilAttachment = nullptr;
                }
            }

            if (subpass.pDepthStencilAttachment)
            {
                const auto &reference = *subpass.pDepthStencilAttachment;

                attachment_descriptions[reference.attachment].finalLayout = reference.layout;
            }

            if (subpass.pResolveAttachments)
            {
                for (size_t k = 0U; k < subpass.colorAttachmentCount; ++k)
                {
                    const auto &reference = subpass.pResolveAttachments[k];

                    attachment_descriptions[reference.attachment].finalLayout = reference.layout;
                }
            }

            if (const auto depth_resolve = GetDepthResolveReference(subpass))
            {
                attachment_descriptions[depth_resolve->attachment].finalLayout = depth_resolve->layout;
            }
        }
    }

    template <typename T>
    std::vector<T> GetSubpassDependencies(const size_t m_SubpassCount)
    {
        std::vector<T> dependencies(m_SubpassCount - 1);

        if (m_SubpassCount > 1)
        {
            for (uint32_t i = 0; i < ToUint32_t(dependencies.size()); ++i)
            {
                // Transition input attachments from color attachment to shader read
                dependencies[i].srcSubpass = i;
                dependencies[i].dstSubpass = i + 1;
                dependencies[i].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependencies[i].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                dependencies[i].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencies[i].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                dependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            }
        }

        return dependencies;
    }

    template <typename T>
    T GetAttachmentReference(const uint32_t attachment, const VkImageLayout layout)
    {
        T reference{};
        SetStructureType(reference);

        reference.attachment = attachment;
        reference.layout = layout;

        return reference;
    }

    template <typename T_SubpassDescription, typename T_AttachmentDescription, typename T_AttachmentReference, typename T_SubpassDependency, typename T_RenderPassCreateInfo>
    void RenderPass::CreateRenderpass(const std::vector<Attachment> &attachments, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<SubpassInfo> &subpasses)
    {
        auto attachment_descriptions = GetAttachmentDescriptions<T_AttachmentDescription>(attachments, load_store_infos);

        // Store attachments for every subpass
        std::vector<std::vector<T_AttachmentReference>> input_attachments{m_SubpassCount};
        std::vector<std::vector<T_AttachmentReference>> color_attachments{m_SubpassCount};
        std::vector<std::vector<T_AttachmentReference>> depth_stencil_attachments{m_SubpassCount};
        std::vector<std::vector<T_AttachmentReference>> color_resolve_attachments{m_SubpassCount};
        std::vector<std::vector<T_AttachmentReference>> depth_resolve_attachments{m_SubpassCount};

        for (size_t i = 0; i < subpasses.size(); ++i)
        {
            auto &subpass = subpasses[i];

            // Fill color attachments references
            for (auto o_attachment : subpass.output_attachments)
            {
                auto initial_layout = attachments[o_attachment].initial_layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : attachments[o_attachment].initial_layout;
                auto &description = attachment_descriptions[o_attachment];
                if (!IsDepthStencilFormat(description.format))
                {
                    color_attachments[i].push_back(GetAttachmentReference<T_AttachmentReference>(o_attachment, initial_layout));
                }
            }

            // Fill input attachments references
            for (auto i_attachment : subpass.input_attachments)
            {
                auto default_layout = IsDepthStencilFormat(attachment_descriptions[i_attachment].format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                auto initial_layout = attachments[i_attachment].initial_layout == VK_IMAGE_LAYOUT_UNDEFINED ? default_layout : attachments[i_attachment].initial_layout;
                input_attachments[i].push_back(GetAttachmentReference<T_AttachmentReference>(i_attachment, initial_layout));
            }

            for (auto r_attachment : subpass.color_resolve_attachments)
            {
                auto initial_layout = attachments[r_attachment].initial_layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : attachments[r_attachment].initial_layout;
                color_resolve_attachments[i].push_back(GetAttachmentReference<T_AttachmentReference>(r_attachment, initial_layout));
            }

            if (!subpass.disable_depth_stencil_attachment)
            {
                // Assumption: depth stencil attachment appears in the list before any depth stencil resolve attachment
                auto it = find_if(attachments.begin(), attachments.end(), [](const Attachment attachment)
                                  { return IsDepthStencilFormat(attachment.format); });
                if (it != attachments.end())
                {
                    auto i_depth_stencil = ToUint32_t(std::distance(attachments.begin(), it));
                    auto initial_layout = it->initial_layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : it->initial_layout;
                    depth_stencil_attachments[i].push_back(GetAttachmentReference<T_AttachmentReference>(i_depth_stencil, initial_layout));

                    if (subpass.depth_stencil_resolve_mode != VK_RESOLVE_MODE_NONE)
                    {
                        auto i_depth_stencil_resolve = subpass.depth_stencil_resolve_attachment;
                        initial_layout = attachments[i_depth_stencil_resolve].initial_layout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : attachments[i_depth_stencil_resolve].initial_layout;
                        depth_resolve_attachments[i].push_back(GetAttachmentReference<T_AttachmentReference>(i_depth_stencil_resolve, initial_layout));
                    }
                }
            }
        }

        std::vector<T_SubpassDescription> subpass_descriptions;
        subpass_descriptions.reserve(m_SubpassCount);
        VkSubpassDescriptionDepthStencilResolveKHR depth_resolve{};
        for (size_t i = 0; i < subpasses.size(); ++i)
        {
            auto &subpass = subpasses[i];

            T_SubpassDescription subpass_description{};
            SetStructureType(subpass_description);
            subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            subpass_description.pInputAttachments = input_attachments[i].empty() ? nullptr : input_attachments[i].data();
            subpass_description.inputAttachmentCount = ToUint32_t(input_attachments[i].size());

            subpass_description.pColorAttachments = color_attachments[i].empty() ? nullptr : color_attachments[i].data();
            subpass_description.colorAttachmentCount = ToUint32_t(color_attachments[i].size());

            subpass_description.pResolveAttachments = color_resolve_attachments[i].empty() ? nullptr : color_resolve_attachments[i].data();

            subpass_description.pDepthStencilAttachment = nullptr;
            if (!depth_stencil_attachments[i].empty())
            {
                subpass_description.pDepthStencilAttachment = depth_stencil_attachments[i].data();

                if (!depth_resolve_attachments[i].empty())
                {
                    // If the pNext list of VkSubpassDescription2 includes a VkSubpassDescriptionDepthStencilResolve structure,
                    // then that structure describes multisample resolve operations for the depth/stencil attachment in a subpass
                    depth_resolve.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE_KHR;
                    depth_resolve.depthResolveMode = subpass.depth_stencil_resolve_mode;
                    SetPointerNext(subpass_description, depth_resolve, depth_resolve_attachments[i][0]);

                    auto &reference = depth_resolve_attachments[i][0];
                    // Set it only if not defined yet
                    if (attachment_descriptions[reference.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                    {
                        attachment_descriptions[reference.attachment].initialLayout = reference.layout;
                    }
                }
            }

            subpass_descriptions.push_back(subpass_description);
        }

        // Default subpass
        if (subpasses.empty())
        {
            T_SubpassDescription subpass_description{};
            SetStructureType(subpass_description);
            subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            uint32_t default_depth_stencil_attachment{VK_ATTACHMENT_UNUSED};

            for (uint32_t k = 0U; k < ToUint32_t(attachment_descriptions.size()); ++k)
            {
                if (IsDepthStencilFormat(attachments[k].format))
                {
                    if (default_depth_stencil_attachment == VK_ATTACHMENT_UNUSED)
                    {
                        default_depth_stencil_attachment = k;
                    }
                    continue;
                }

                color_attachments[0].push_back(GetAttachmentReference<T_AttachmentReference>(k, VK_IMAGE_LAYOUT_GENERAL));
            }

            subpass_description.pColorAttachments = color_attachments[0].data();

            if (default_depth_stencil_attachment != VK_ATTACHMENT_UNUSED)
            {
                depth_stencil_attachments[0].push_back(GetAttachmentReference<T_AttachmentReference>(default_depth_stencil_attachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));

                subpass_description.pDepthStencilAttachment = depth_stencil_attachments[0].data();
            }

            subpass_descriptions.push_back(subpass_description);
        }

        SetAttachmentLayouts<T_SubpassDescription, T_AttachmentDescription, T_AttachmentReference>(subpass_descriptions, attachment_descriptions);

        m_ColorOutputCount.reserve(m_SubpassCount);
        for (size_t i = 0; i < m_SubpassCount; i++)
        {
            m_ColorOutputCount.push_back(ToUint32_t(color_attachments[i].size()));
        }

        const auto &subpass_dependencies = GetSubpassDependencies<T_SubpassDependency>(m_SubpassCount);

        T_RenderPassCreateInfo create_info{};
        SetStructureType(create_info);
        create_info.attachmentCount = ToUint32_t(attachment_descriptions.size());
        create_info.pAttachments = attachment_descriptions.data();
        create_info.subpassCount = ToUint32_t(subpass_descriptions.size());
        create_info.pSubpasses = subpass_descriptions.data();
        create_info.dependencyCount = ToUint32_t(subpass_dependencies.size());
        create_info.pDependencies = subpass_dependencies.data();

        auto result = CreateVkRenderpass(m_Device.GetHandle(), create_info, &m_Handle);

        if (result != VK_SUCCESS)
        {
            throw VulkanException{result, "Cannot create RenderPass"};
        }
    }

    RenderPass::RenderPass(Device &device, const std::vector<Attachment> &attachments, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<SubpassInfo> &subpasses)
        : m_Device{device},
          m_SubpassCount{std::max<size_t>(1, subpasses.size())}, // At least 1 subpass
          m_ColorOutputCount{}
    {
        if (m_Device.IsExtensionEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME))
            CreateRenderpass<VkSubpassDescription2KHR, VkAttachmentDescription2KHR, VkAttachmentReference2KHR, VkSubpassDependency2KHR, VkRenderPassCreateInfo2KHR>(attachments, load_store_infos, subpasses);
        else
            CreateRenderpass<VkSubpassDescription, VkAttachmentDescription, VkAttachmentReference, VkSubpassDependency, VkRenderPassCreateInfo>(attachments, load_store_infos, subpasses);
    }

    RenderPass::RenderPass(RenderPass &&other)
        : m_Device{other.m_Device},
          m_Handle{other.m_Handle},
          m_SubpassCount{other.m_SubpassCount},
          m_ColorOutputCount{other.m_ColorOutputCount}
    {
        other.m_Handle = VK_NULL_HANDLE;
    }

    RenderPass::~RenderPass()
    {
        if (m_Handle != VK_NULL_HANDLE)
            vkDestroyRenderPass(m_Device.GetHandle(), m_Handle, nullptr);
    }

    const uint32_t RenderPass::GetColorOutputCount(uint32_t subpass_index) const
    {
        return m_ColorOutputCount[subpass_index];
    }

    const VkExtent2D RenderPass::GetRenderAreaGranularity() const
    {
        VkExtent2D render_area_granularity = {};
        vkGetRenderAreaGranularity(m_Device.GetHandle(), m_Handle, &render_area_granularity);

        return render_area_granularity;
    }
}
