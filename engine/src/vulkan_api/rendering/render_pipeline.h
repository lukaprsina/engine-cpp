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
    };
}
