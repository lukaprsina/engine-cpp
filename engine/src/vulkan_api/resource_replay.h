#pragma once

#include "vulkan_api/resource_record.h"

namespace engine
{
    class ResourceCache;

    class ResourceReplay
    {
    public:
        ResourceReplay();

        void Play(ResourceCache &resource_cache, ResourceRecord &recorder);

    protected:
        void CreateShaderModule(ResourceCache &resource_cache, std::istringstream &stream);
        void CreatePipelineLayout(ResourceCache &resource_cache, std::istringstream &stream);
        void CreateRenderPass(ResourceCache &resource_cache, std::istringstream &stream);
        void CreateGraphicsPipeline(ResourceCache &resource_cache, std::istringstream &stream);

    private:
        using ResourceFunc = std::function<void(ResourceCache &, std::istringstream &)>;
        std::unordered_map<ResourceType, ResourceFunc> m_StreamResources;
        std::vector<ShaderModule *> m_ShaderModules;
        std::vector<PipelineLayout *> m_PipelineLayouts;
        std::vector<const RenderPass *> m_RenderPasses;
        std::vector<const GraphicsPipeline *> m_GraphicsPipelines;
    };
} // namespace engine
