#include "vulkan_api/core/framebuffer.h"

#include "vulkan_api/device.h"
#include "vulkan_api/render_target.h"
#include "vulkan_api/core/render_pass.h"

namespace engine
{
    Framebuffer::Framebuffer(Device &device, const RenderTarget &render_target, const RenderPass &render_pass)
        : m_Device(device), m_Extent(render_target.GetExtent())
    {
        std::vector<VkImageView> attachments;

        for (auto &view : render_target.GetViews())
            attachments.emplace_back(view.GetHandle());

        VkFramebufferCreateInfo create_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};

        create_info.renderPass = render_pass.GetHandle();
        create_info.attachmentCount = ToUint32_t(attachments.size());
        create_info.pAttachments = attachments.data();
        create_info.width = m_Extent.width;
        create_info.height = m_Extent.height;
        create_info.layers = 1;

        auto result = vkCreateFramebuffer(device.GetHandle(), &create_info, nullptr, &m_Handle);

        if (result != VK_SUCCESS)
            throw VulkanException{result, "Cannot create Framebuffer"};
    }

    Framebuffer::~Framebuffer()
    {
        if (m_Handle != VK_NULL_HANDLE)
            vkDestroyFramebuffer(m_Device.GetHandle(), m_Handle, nullptr);
    }

    Framebuffer::Framebuffer(Framebuffer &&other)
        : m_Device{other.m_Device},
          m_Handle{other.m_Handle},
          m_Extent{other.m_Extent}
    {
        other.m_Handle = VK_NULL_HANDLE;
    }
}
