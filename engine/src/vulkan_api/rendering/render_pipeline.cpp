#include "vulkan_api/rendering/render_pipeline.h"

#include "vulkan_api/command_buffer.h"
#include "vulkan_api/render_target.h"
#include "vulkan_api/subpasses/subpass.h"

namespace engine
{
    RenderPipeline::RenderPipeline(std::vector<std::unique_ptr<Subpass>> &&subpasses)
    {
    }

    RenderPipeline::~RenderPipeline()
    {
    }

    void RenderPipeline::Draw(CommandBuffer &command_buffer, RenderTarget &render_target, VkSubpassContents contents)
    {
    }
}
