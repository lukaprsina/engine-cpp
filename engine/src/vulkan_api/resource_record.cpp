/* Copyright (c) 2019-2020, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vulkan_api/resource_record.h"

#include "vulkan_api/core/pipeline.h"
#include "vulkan_api/core/pipeline_layout.h"
#include "vulkan_api/core/render_pass.h"
#include "vulkan_api/render_target.h"
#include "renderer/shader.h"
#include "vulkan_api/resource_cache.h"

#include "common/helpers.h"

namespace engine
{
    namespace
    {
        inline void WriteSubpassInfo(std::ostringstream &os, const std::vector<SubpassInfo> &value)
        {
            Write(os, value.size());
            for (const SubpassInfo &item : value)
            {
                Write(os, item.input_attachments);
                Write(os, item.output_attachments);
            }
        }

        inline void WriteProcesses(std::ostringstream &os, const std::vector<std::string> &value)
        {
            Write(os, value.size());
            for (const std::string &item : value)
            {
                Write(os, item);
            }
        }
    }

    void ResourceRecord::SetData(const std::vector<uint8_t> &data)
    {
        m_Stream.str(std::string{data.begin(), data.end()});
    }

    std::vector<uint8_t> ResourceRecord::GetData()
    {
        std::string str = m_Stream.str();

        return std::vector<uint8_t>{str.begin(), str.end()};
    }

    const std::ostringstream &ResourceRecord::GetStream()
    {
        return m_Stream;
    }

    size_t ResourceRecord::RegisterShaderModule(VkShaderStageFlagBits stage, const ShaderSource &glsl_source, const std::string &entry_point, const ShaderVariant &shader_variant)
    {
        m_ShaderModuleIndices.push_back(m_ShaderModuleIndices.size());

        Write(m_Stream, ResourceType::ShaderModule, stage, glsl_source.GetFileContent(), entry_point, shader_variant.GetPreamble());

        WriteProcesses(m_Stream, shader_variant.GetProcesses());

        return m_ShaderModuleIndices.back();
    }

    size_t ResourceRecord::RegisterPipelineLayout(const std::vector<ShaderModule *> &shader_modules)
    {
        m_PipelineLayoutIndices.push_back(m_PipelineLayoutIndices.size());

        std::vector<size_t> shader_indices(shader_modules.size());
        std::transform(shader_modules.begin(), shader_modules.end(), shader_indices.begin(),
                       [this](ShaderModule *shader_module)
                       { return m_ShaderModuleToIndex.at(shader_module); });

        Write(m_Stream,
              ResourceType::PipelineLayout,
              shader_indices);

        return m_PipelineLayoutIndices.back();
    }

    size_t ResourceRecord::RegisterRenderPass(const std::vector<Attachment> &attachments, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<SubpassInfo> &subpasses)
    {
        m_RenderPassIndices.push_back(m_RenderPassIndices.size());

        Write(m_Stream,
              ResourceType::RenderPass,
              attachments,
              load_store_infos);

        WriteSubpassInfo(m_Stream, subpasses);

        return m_RenderPassIndices.back();
    }

    size_t ResourceRecord::RegisterGraphicsPipeline(VkPipelineCache /*pipeline_cache*/, PipelineState &pipeline_state)
    {
        m_GraphicsPipelineIndices.push_back(m_GraphicsPipelineIndices.size());

        auto &pipeline_layout = pipeline_state.GetPipelineLayout();
        auto render_pass = pipeline_state.GetRenderPass();

        Write(m_Stream,
              ResourceType::GraphicsPipeline,
              m_PipelineLayoutToIndex.at(&pipeline_layout),
              m_RenderPassToIndex.at(render_pass),
              pipeline_state.GetSubpassIndex());

        auto &specialization_constant_state = pipeline_state.GetSpecializationConstantState().GetSpecializationConstantState();

        Write(m_Stream,
              specialization_constant_state);

        auto &vertex_input_state = pipeline_state.GetVertexInputState();

        Write(m_Stream,
              vertex_input_state.attributes,
              vertex_input_state.bindings);

        Write(m_Stream,
              pipeline_state.GetInputAssemblyState(),
              pipeline_state.GetRasterizationState(),
              pipeline_state.GetViewportState(),
              pipeline_state.GetMultisampleState(),
              pipeline_state.GetDepthStencilState());

        auto &color_blend_state = pipeline_state.GetColorBlendState();

        Write(m_Stream,
              color_blend_state.logic_op,
              color_blend_state.logic_op_enable,
              color_blend_state.attachments);

        return m_GraphicsPipelineIndices.back();
    }

    void ResourceRecord::SetShaderModule(size_t index, const ShaderModule &shader_module)
    {
        m_ShaderModuleToIndex[&shader_module] = index;
    }

    void ResourceRecord::SetPipelineLayout(size_t index, const PipelineLayout &pipeline_layout)
    {
        m_PipelineLayoutToIndex[&pipeline_layout] = index;
    }

    void ResourceRecord::SetRenderPass(size_t index, const RenderPass &render_pass)
    {
        m_RenderPassToIndex[&render_pass] = index;
    }

    void ResourceRecord::SetGraphicsPipeline(size_t index, const GraphicsPipeline &graphics_pipeline)
    {
        m_GraphicsPipelineToIndex[&graphics_pipeline] = index;
    }

}
