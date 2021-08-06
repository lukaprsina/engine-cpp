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
            std::shared_ptr<Framebuffer> load(const RenderTarget &render_target, const RenderPass &render_pass) const
            {
                return std::shared_ptr<Framebuffer>(new Framebuffer{m_Device, render_target, render_pass});
            }
        };

        struct State
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

    private:
        Device &m_Device;
        VkPipelineCache m_PipelineCache{VK_NULL_HANDLE};

        State m_State;
    };
}
/*
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

struct Loaders
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

const State &GetState() const { return m_State; }
const Mutexes &GetMutexes() const { return m_Mutexes; }
const Loaders &GetLoaders() const { return m_Loaders; }
* /