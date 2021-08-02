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

        void Draw(CommandBuffer &command_buffer, RenderTarget &render_target,
                  VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
    };
}
