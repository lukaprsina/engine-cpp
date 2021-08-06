#pragma once

#include "renderer/shader.h"
#include "vulkan_api/core/pipeline_layout.h"
#include "vulkan_api/core/descriptor_set_layout.h"
#include "vulkan_api/core/descriptor_pool.h"
#include "vulkan_api/core/render_pass.h"
#include "vulkan_api/core/descriptor_set.h"
#include "vulkan_api/core/framebuffer.h"
#include "vulkan_api/resource_record.h"
#include "vulkan_api/resource_replay.h"
#include "vulkan_api/core/pipeline.h"

namespace engine
{
    class Device;

    namespace core
    {
        class ImageView;
    }

    class ResourceCache
    {
    public:
        ResourceCache(Device &device);
        ResourceCache(const ResourceCache &) = delete;
        ResourceCache(ResourceCache &&) = delete;
        ~ResourceCache();
        ResourceCache &operator=(const ResourceCache &) = delete;
        ResourceCache &operator=(ResourceCache &&) = delete;

        ShaderModule &RequestShaderModule(VkShaderStageFlagBits stage, const ShaderSource &glsl_source, const ShaderVariant &shader_variant = {});

        PipelineLayout &RequestPipelineLayout(const std::vector<ShaderModule *> &shader_modules);

        DescriptorSetLayout &RequestDescriptorSetLayout(const uint32_t set_index,
                                                        const std::vector<ShaderModule *> &shader_modules,
                                                        const std::vector<ShaderResource> &set_resources);

        GraphicsPipeline &RequestGraphicsPipeline(PipelineState &pipeline_state);

        ComputePipeline &RequestComputePipeline(PipelineState &pipeline_state);

        DescriptorSet &RequestDescriptorSet(DescriptorSetLayout &descriptor_set_layout,
                                            const BindingMap<VkDescriptorBufferInfo> &buffer_infos,
                                            const BindingMap<VkDescriptorImageInfo> &image_infos);

        RenderPass &RequestRenderPass(const std::vector<Attachment> &attachments,
                                      const std::vector<LoadStoreInfo> &load_store_infos,
                                      const std::vector<SubpassInfo> &subpasses);

        Framebuffer &RequestFramebuffer(const RenderTarget &render_target,
                                        const RenderPass &render_pass);

        void SetPipelineCache(VkPipelineCache pipeline_cache);
        void ClearPipelines();
        void UpdateDescriptorSets(const std::vector<core::ImageView> &old_views, const std::vector<core::ImageView> &new_views);
        void ClearFramebuffers();
        void Clear();

        struct State
        {
            std::unordered_map<std::size_t, ShaderModule> shader_modules;
            std::unordered_map<std::size_t, PipelineLayout> pipeline_layouts;
            std::unordered_map<std::size_t, DescriptorSetLayout> descriptor_set_layouts;
            std::unordered_map<std::size_t, DescriptorPool> descriptor_pools;
            std::unordered_map<std::size_t, RenderPass> render_passes;
            std::unordered_map<std::size_t, GraphicsPipeline> graphics_pipelines;
            std::unordered_map<std::size_t, ComputePipeline> compute_pipelines;
            std::unordered_map<std::size_t, DescriptorSet> descriptor_sets;
            std::unordered_map<std::size_t, Framebuffer> framebuffers;
        };

        struct Mutexes
        {
            std::mutex shader_module;
            std::mutex pipeline_layout;
            std::mutex descriptor_set_layout;

            std::mutex render_pass;
            std::mutex graphics_pipeline;
            std::mutex compute_pipeline;
            std::mutex descriptor_set;
            std::mutex framebuffer;
        };

    private:
        Device &m_Device;
        VkPipelineCache m_PipelineCache{VK_NULL_HANDLE};
        ResourceRecord m_Recorder;
        ResourceReplay m_Replayer;
        State m_State;
        Mutexes m_Mutexes;
    };
}
