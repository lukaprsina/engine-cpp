#pragma once

#include "vulkan_api/rendering/pipeline_state.h"

#include <sstream>

namespace engine
{
    class GraphicsPipeline;
    class PipelineLayout;
    class RenderPass;
    class ShaderModule;
    class ShaderSource;
    class ShaderVariant;
    class Attachment;
    struct LoadStoreInfo;
    struct SubpassInfo;
    class PipelineState;

    enum class ResourceType
    {
        ShaderModule,
        PipelineLayout,
        RenderPass,
        GraphicsPipeline
    };

    class ResourceRecord
    {
    public:
        void SetData(const std::vector<uint8_t> &data);
        std::vector<uint8_t> GetData();
        const std::ostringstream &GetStream();

        size_t RegisterShaderModule(VkShaderStageFlagBits stage,
                                    const ShaderSource &glsl_source,
                                    const std::string &entry_point,
                                    const ShaderVariant &shader_variant);

        size_t RegisterPipelineLayout(const std::vector<ShaderModule *> &shader_modules);

        size_t RegisterRenderPass(const std::vector<Attachment> &attachments,
                                  const std::vector<LoadStoreInfo> &load_store_infos,
                                  const std::vector<SubpassInfo> &subpasses);

        size_t RegisterGraphicsPipeline(VkPipelineCache pipeline_cache,
                                        PipelineState &pipeline_state);

        void SetShaderModule(size_t index, const ShaderModule &shader_module);
        void SetPipelineLayout(size_t index, const PipelineLayout &pipeline_layout);
        void SetRenderPass(size_t index, const RenderPass &render_pass);
        void SetGraphicsPipeline(size_t index, const GraphicsPipeline &graphics_pipeline);

    private:
        std::ostringstream m_Stream;
        std::vector<size_t> m_ShaderModuleIndices;
        std::vector<size_t> m_PipelineLayoutIndices;
        std::vector<size_t> m_RenderPassIndices;
        std::vector<size_t> m_GraphicsPipelineIndices;
        std::unordered_map<const ShaderModule *, size_t> m_ShaderModuleToIndex;
        std::unordered_map<const PipelineLayout *, size_t> m_PipelineLayoutToIndex;
        std::unordered_map<const RenderPass *, size_t> m_RenderPassToIndex;
        std::unordered_map<const GraphicsPipeline *, size_t> m_GraphicsPipelineToIndex;
    };
}
