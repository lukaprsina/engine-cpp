#pragma once

namespace engine
{
    class CommandBuffer;
    class RenderTarget;
    class Subpass;
    class Device;
    class RenderContext;

    class RenderPipeline
    {
    public:
        RenderPipeline(Device &device, std::vector<std::unique_ptr<Subpass>> &&subpasses = {});
        ~RenderPipeline() = default;

        void AddSubpass(std::unique_ptr<Subpass> &&subpass);

        void Draw(RenderContext &render_context, CommandBuffer &command_buffer, RenderTarget &render_target,
                  VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

    private:
        Device &m_Device;
        std::vector<std::unique_ptr<Subpass>> m_Subpasses{};
        std::vector<LoadStoreInfo> m_LoadStore{};
        std::vector<VkClearValue> m_ClearValue{};
        size_t m_ActiveSubpassIndex{0};
    };
}
