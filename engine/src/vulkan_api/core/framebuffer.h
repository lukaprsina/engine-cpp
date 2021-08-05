#pragma once

namespace engine
{
    class Device;
    class RenderTarget;
    class RenderPass;

    class Framebuffer
    {
    public:
        Framebuffer(Device &device, const RenderTarget &render_target, const RenderPass &render_pass);
        ~Framebuffer();
        Framebuffer(Framebuffer &&other);

        VkFramebuffer GetHandle() const { return m_Handle; }
        const VkExtent2D &GetExtent() const { return m_Extent; }

    private:
        Device &m_Device;
        VkFramebuffer m_Handle{VK_NULL_HANDLE};
        VkExtent2D m_Extent{};
    };
}
