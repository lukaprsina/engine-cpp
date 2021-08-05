#pragma once

namespace engine
{
    class CommandBuffer;
    class RenderTarget;
    class Subpass;

    class RenderPipeline
    {
    public:
        RenderPipeline(std::vector<std::unique_ptr<Subpass>> &&subpasses = {});
        ~RenderPipeline();

        void AddSubpass(std::unique_ptr<Subpass> &&subpass);

        void Draw(CommandBuffer &command_buffer, RenderTarget &render_target,
                  VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

    private:
        std::vector<std::unique_ptr<Subpass>> m_Subpasses{};
        std::vector<LoadStoreInfo> m_LoadStore = std::vector<LoadStoreInfo>(2);
        std::vector<VkClearValue> m_ClearValue = std::vector<VkClearValue>(2);
        size_t m_ActiveSubpassIndex{0};
    };
}
