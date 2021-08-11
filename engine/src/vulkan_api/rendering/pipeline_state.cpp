#include "vulkan_api/rendering/pipeline_state.h"

bool operator==(const VkVertexInputAttributeDescription &lhs, const VkVertexInputAttributeDescription &rhs)
{
    return std::tie(lhs.binding, lhs.format, lhs.location, lhs.offset) == std::tie(rhs.binding, rhs.format, rhs.location, rhs.offset);
}

bool operator==(const VkVertexInputBindingDescription &lhs, const VkVertexInputBindingDescription &rhs)
{
    return std::tie(lhs.binding, lhs.inputRate, lhs.stride) == std::tie(rhs.binding, rhs.inputRate, rhs.stride);
}

bool operator==(const engine::ColorBlendAttachmentState &lhs, const engine::ColorBlendAttachmentState &rhs)
{
    return std::tie(lhs.alpha_blend_op, lhs.blend_enable, lhs.color_blend_op, lhs.color_write_mask, lhs.dst_alpha_blend_factor, lhs.dst_color_blend_factor, lhs.src_alpha_blend_factor, lhs.src_color_blend_factor) ==
           std::tie(rhs.alpha_blend_op, rhs.blend_enable, rhs.color_blend_op, rhs.color_write_mask, rhs.dst_alpha_blend_factor, rhs.dst_color_blend_factor, rhs.src_alpha_blend_factor, rhs.src_color_blend_factor);
}

bool operator!=(const engine::StencilOpState &lhs, const engine::StencilOpState &rhs)
{
    return std::tie(lhs.compare_op, lhs.depth_fail_op, lhs.fail_op, lhs.pass_op) != std::tie(rhs.compare_op, rhs.depth_fail_op, rhs.fail_op, rhs.pass_op);
}

bool operator!=(const engine::VertexInputState &lhs, const engine::VertexInputState &rhs)
{
    return lhs.attributes != rhs.attributes || lhs.bindings != rhs.bindings;
}

bool operator!=(const engine::InputAssemblyState &lhs, const engine::InputAssemblyState &rhs)
{
    return std::tie(lhs.primitive_restart_enable, lhs.topology) != std::tie(rhs.primitive_restart_enable, rhs.topology);
}

bool operator!=(const engine::RasterizationState &lhs, const engine::RasterizationState &rhs)
{
    return std::tie(lhs.cull_mode, lhs.depth_bias_enable, lhs.depth_clamp_enable, lhs.front_face, lhs.front_face, lhs.polygon_mode, lhs.rasterizer_discard_enable) !=
           std::tie(rhs.cull_mode, rhs.depth_bias_enable, rhs.depth_clamp_enable, rhs.front_face, rhs.front_face, rhs.polygon_mode, rhs.rasterizer_discard_enable);
}

bool operator!=(const engine::ViewportState &lhs, const engine::ViewportState &rhs)
{
    return lhs.viewport_count != rhs.viewport_count || lhs.scissor_count != rhs.scissor_count;
}

bool operator!=(const engine::MultisampleState &lhs, const engine::MultisampleState &rhs)
{
    return std::tie(lhs.alpha_to_coverage_enable, lhs.alpha_to_one_enable, lhs.min_sample_shading, lhs.rasterization_samples, lhs.sample_mask, lhs.sample_shading_enable) !=
           std::tie(rhs.alpha_to_coverage_enable, rhs.alpha_to_one_enable, rhs.min_sample_shading, rhs.rasterization_samples, rhs.sample_mask, rhs.sample_shading_enable);
}

bool operator!=(const engine::DepthStencilState &lhs, const engine::DepthStencilState &rhs)
{
    return std::tie(lhs.depth_bounds_test_enable, lhs.depth_compare_op, lhs.depth_test_enable, lhs.depth_write_enable, lhs.stencil_test_enable) !=
               std::tie(rhs.depth_bounds_test_enable, rhs.depth_compare_op, rhs.depth_test_enable, rhs.depth_write_enable, rhs.stencil_test_enable) ||
           lhs.back != rhs.back || lhs.front != rhs.front;
}

bool operator!=(const engine::ColorBlendState &lhs, const engine::ColorBlendState &rhs)
{
    return std::tie(lhs.logic_op, lhs.logic_op_enable) != std::tie(rhs.logic_op, rhs.logic_op_enable) ||
           lhs.attachments.size() != rhs.attachments.size() ||
           !std::equal(lhs.attachments.begin(), lhs.attachments.end(), rhs.attachments.begin(),
                       [](const engine::ColorBlendAttachmentState &lhs, const engine::ColorBlendAttachmentState &rhs)
                       {
                           return lhs == rhs;
                       });
}

namespace engine
{

    void SpecializationConstantState::SetConstant(uint32_t constant_id, const std::vector<uint8_t> &value)
    {
        auto data = m_SpecializationConstantState.find(constant_id);

        if (data != m_SpecializationConstantState.end() && data->second == value)
            return;

        m_Dirty = true;

        m_SpecializationConstantState[constant_id] = value;
    }

    void PipelineState::Reset()
    {
        ClearDirty();
        m_PipelineLayout = nullptr;
        m_RenderPass = nullptr;
        m_SpecializationConstantState.Reset();
        m_VertexInputState = {};
        m_InputAssemblyState = {};
        m_RasterizationState = {};
        m_ViewportState = {};
        m_MultisampleState = {};
        m_DepthStencilState = {};
        m_ColorBlendState = {};
        m_SubpassIndex = {0U};
    }

    void PipelineState::SetPipelineLayout(PipelineLayout &new_pipeline_layout)
    {
        if (m_PipelineLayout)
        {
            if (m_PipelineLayout->GetHandle() != new_pipeline_layout.GetHandle())
            {
                m_PipelineLayout = &new_pipeline_layout;

                m_Dirty = true;
            }
        }
        else
        {
            m_PipelineLayout = &new_pipeline_layout;

            m_Dirty = true;
        }
    }

    void PipelineState::SetRenderPass(const RenderPass &new_render_pass)
    {
        if (m_RenderPass)
        {
            if (m_RenderPass->GetHandle() != new_render_pass.GetHandle())
            {
                m_RenderPass = &new_render_pass;

                m_Dirty = true;
            }
        }
        else
        {
            m_RenderPass = &new_render_pass;

            m_Dirty = true;
        }
    }

    void PipelineState::SetSpecializationConstant(uint32_t constant_id, const std::vector<uint8_t> &data)
    {
        m_SpecializationConstantState.SetConstant(constant_id, data);

        if (m_SpecializationConstantState.IsDirty())
        {
            m_Dirty = true;
        }
    }

    void PipelineState::SetVertexInputState(const VertexInputState &new_vertex_input_state)
    {
        if (m_VertexInputState != new_vertex_input_state)
        {
            m_VertexInputState = new_vertex_input_state;

            m_Dirty = true;
        }
    }

    void PipelineState::SetInputAssemblyState(const InputAssemblyState &new_input_assembly_state)
    {
        if (m_InputAssemblyState != new_input_assembly_state)
        {
            m_InputAssemblyState = new_input_assembly_state;

            m_Dirty = true;
        }
    }

    void PipelineState::SetRasterizationState(const RasterizationState &new_rasterization_state)
    {
        if (m_RasterizationState != new_rasterization_state)
        {
            m_RasterizationState = new_rasterization_state;

            m_Dirty = true;
        }
    }

    void PipelineState::SetViewportState(const ViewportState &new_viewport_state)
    {
        if (m_ViewportState != new_viewport_state)
        {
            m_ViewportState = new_viewport_state;

            m_Dirty = true;
        }
    }

    void PipelineState::SetMultisampleState(const MultisampleState &new_multisample_state)
    {
        if (m_MultisampleState != new_multisample_state)
        {
            m_MultisampleState = new_multisample_state;

            m_Dirty = true;
        }
    }

    void PipelineState::SetDepthStencilState(const DepthStencilState &new_depth_stencil_state)
    {
        if (m_DepthStencilState != new_depth_stencil_state)
        {
            m_DepthStencilState = new_depth_stencil_state;

            m_Dirty = true;
        }
    }

    void PipelineState::SetColorBlendState(const ColorBlendState &new_color_blend_state)
    {
        if (m_ColorBlendState != new_color_blend_state)
        {
            m_ColorBlendState = new_color_blend_state;

            m_Dirty = true;
        }
    }

    void PipelineState::SetSubpassIndex(uint32_t new_subpass_index)
    {
        if (m_SubpassIndex != new_subpass_index)
        {
            m_SubpassIndex = new_subpass_index;

            m_Dirty = true;
        }
    }

    void PipelineState::ClearDirty()
    {
        m_Dirty = false;
        m_SpecializationConstantState.ClearDirty();
    }
}
