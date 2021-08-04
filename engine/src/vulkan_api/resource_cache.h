#pragma once

#include "renderer/shader.h"
#include "vulkan_api/core/pipeline_layout.h"
#include "vulkan_api/core/descriptor_set_layout.h"
#include "vulkan_api/core/descriptor_pool.h"
#include "vulkan_api/core/render_pass.h"
#include "vulkan_api/core/graphics_pipeline.h"
#include "vulkan_api/core/compute_pipeline.h"
#include "vulkan_api/core/descriptor_set.h"
#include "vulkan_api/core/framebuffer.h"

#include <entt/entt.hpp>

namespace engine
{
    class Device;

    struct ResourceCacheState
    {
        entt::resource_cache<ShaderSource> shader_modules;
        entt::resource_cache<PipelineLayout> pipeline_layouts;
        entt::resource_cache<DescriptorSetLayout> descriptor_set_layouts;
        entt::resource_cache<DescriptorPool> descriptor_pools;
        entt::resource_cache<RenderPass> render_passes;
        entt::resource_cache<GraphicsPipeline> graphics_pipelines;
        entt::resource_cache<ComputePipeline> compute_pipelines;
        entt::resource_cache<DescriptorSet> descriptor_sets;
        entt::resource_cache<Framebuffer> framebuffers;
    };

    struct ResourceCacheMutexes
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

    struct ShaderSourceLoader : entt::resource_loader<ShaderSourceLoader, ShaderSource>
    {
        std::shared_ptr<ShaderSource> load(const std::string &filename) const
        {
            return std::shared_ptr<ShaderSource>(new ShaderSource{filename});
        }
    };

    struct PipelineLayoutLoader : entt::resource_loader<PipelineLayoutLoader, PipelineLayout>
    {
        std::shared_ptr<PipelineLayout> load() const
        {
            return std::shared_ptr<PipelineLayout>(new PipelineLayout{});
        }
    };

    struct DescriptorSetLayoutLoader : entt::resource_loader<DescriptorSetLayoutLoader, DescriptorSetLayout>
    {
        std::shared_ptr<DescriptorSetLayout> load() const
        {
            return std::shared_ptr<DescriptorSetLayout>(new DescriptorSetLayout{});
        }
    };

    struct RenderPassLoader : entt::resource_loader<RenderPassLoader, RenderPass>
    {
        std::shared_ptr<RenderPass> load() const
        {
            return std::shared_ptr<RenderPass>(new RenderPass{});
        }
    };

    struct GraphicsPipelineLoader : entt::resource_loader<GraphicsPipelineLoader, GraphicsPipeline>
    {
        std::shared_ptr<GraphicsPipeline> load() const
        {
            return std::shared_ptr<GraphicsPipeline>(new GraphicsPipeline{});
        }
    };

    struct ComputePipelineLoader : entt::resource_loader<ComputePipelineLoader, ComputePipeline>
    {
        std::shared_ptr<ComputePipeline> load() const
        {
            return std::shared_ptr<ComputePipeline>(new ComputePipeline{});
        }
    };

    struct DescriptorSetLoader : entt::resource_loader<DescriptorSetLoader, DescriptorSet>
    {
        std::shared_ptr<DescriptorSet> load() const
        {
            return std::shared_ptr<DescriptorSet>(new DescriptorSet{});
        }
    };

    struct FramebufferLoader : entt::resource_loader<FramebufferLoader, Framebuffer>
    {
        std::shared_ptr<Framebuffer> load() const
        {
            return std::shared_ptr<Framebuffer>(new Framebuffer{});
        }
    };

    struct ResourceCacheLoaders
    {
        ShaderSourceLoader shader_module;
        PipelineLayoutLoader pipeline_layout;
        DescriptorSetLayoutLoader descriptor_set_layout;
        RenderPassLoader render_pass;
        GraphicsPipelineLoader graphics_pipeline;
        ComputePipelineLoader compute_pipeline;
        DescriptorSetLoader descriptor_set;
        FramebufferLoader framebuffer;
    };

    class ResourceCache
    {
    public:
        ResourceCache(Device &device);
        ResourceCache(const ResourceCache &) = delete;
        ResourceCache(ResourceCache &&) = delete;
        ~ResourceCache();
        ResourceCache &operator=(const ResourceCache &) = delete;
        ResourceCache &operator=(ResourceCache &&) = delete;

    private:
        Device &m_Device;
        VkPipelineCache m_PipelineCache{VK_NULL_HANDLE};

        ResourceCacheState m_State;
        ResourceCacheMutexes m_Mutexes;
        ResourceCacheLoaders m_Loaders;
    };
}