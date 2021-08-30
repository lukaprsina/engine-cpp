#include "vulkan_api/rendering/render_pipeline.h"

#include "vulkan_api/command_buffer.h"
#include "vulkan_api/render_target.h"
#include "vulkan_api/subpasses/subpass.h"

namespace engine
{
    RenderPipeline::RenderPipeline(std::vector<std::unique_ptr<Subpass>> &&subpasses)
        : m_Subpasses(std::move(subpasses))
    {
        for (auto &subpass : m_Subpasses)
        {
            subpass->Prepare();
        }

        m_ClearValue[0].color = {0.05f, 0.05f, 0.05f, 1.0f};
        m_ClearValue[1].depthStencil = {0.0f, ~0U};
    }

    RenderPipeline::~RenderPipeline()
    {
    }

    void RenderPipeline::AddSubpass(std::unique_ptr<Subpass> &&subpass)
    {
        subpass->Prepare();
        m_Subpasses.emplace_back(std::move(subpass));
    }

    void RenderPipeline::Draw(CommandBuffer &command_buffer, RenderTarget &render_target, VkSubpassContents contents)
    {
        assert(!m_Subpasses.empty() && "Render pipeline should contain at least one sub-pass");

        // Pad clear values if they're less than render target attachments
        while (m_ClearValue.size() < render_target.GetAttachments().size())
            m_ClearValue.push_back({0.0f, 0.0f, 0.0f, 1.0f});

        for (size_t i = 0; i < m_Subpasses.size(); ++i)
        {
            m_ActiveSubpassIndex = i;
            auto &subpass = m_Subpasses[i];
            subpass->UpdateRenderTargetAttachments(render_target);

            if (i == 0)
                command_buffer.BeginRenderPass(render_target, m_LoadStore, m_ClearValue, m_Subpasses, contents);

            else
                command_buffer.NextSubpass();

            subpass->Draw(command_buffer);
        }

        m_ActiveSubpassIndex = 0;
    }
}
