#include "vulkan_api/resource_cache.h"

#include "vulkan_api/device.h"
#include "common/resource_caching.h"

namespace engine
{
    namespace
    {
        template <class T, class... A>
        T &RequestResource(Device &device, ResourceRecord &recorder,
                           std::mutex &mutex, std::unordered_map<std::size_t, T> &resources,
                           A &...args)
        {
            std::lock_guard<std::mutex> guard(mutex);

            auto &res = RequestResource(device, &recorder, resources, args...);

            return res;
        }
    }

    ResourceCache::ResourceCache(Device &device)
        : m_Device(device)
    {
    }

    ResourceCache::~ResourceCache()
    {
    }

    void ResourceCache::SetPipelineCache(VkPipelineCache new_pipeline_cache)
    {
        m_PipelineCache = new_pipeline_cache;
    }

    ShaderModule &ResourceCache::RequestShaderModule(VkShaderStageFlagBits stage, const ShaderSource &glsl_source, const ShaderVariant &shader_variant)
    {
        std::string entry_point{"main"};
        return RequestResource(m_Device, m_Recorder, m_Mutexes.shader_module, m_State.shader_modules, stage, glsl_source, entry_point, shader_variant);
    }

    PipelineLayout &ResourceCache::RequestPipelineLayout(const std::vector<ShaderModule *> &shader_modules)
    {
        return RequestResource(m_Device, m_Recorder, m_Mutexes.pipeline_layout, m_State.pipeline_layouts, shader_modules);
    }

    DescriptorSetLayout &ResourceCache::RequestDescriptorSetLayout(const uint32_t set_index,
                                                                   const std::vector<ShaderModule *> &shader_modules,
                                                                   const std::vector<ShaderResource> &set_resources)
    {
        return RequestResource(m_Device, m_Recorder, m_Mutexes.descriptor_set_layout, m_State.descriptor_set_layouts, set_index, shader_modules, set_resources);
    }

    GraphicsPipeline &ResourceCache::RequestGraphicsPipeline(PipelineState &pipeline_state)
    {
        return RequestResource(m_Device, m_Recorder, m_Mutexes.graphics_pipeline, m_State.graphics_pipelines, m_PipelineCache, pipeline_state);
    }

    ComputePipeline &ResourceCache::RequestComputePipeline(PipelineState &pipeline_state)
    {
        return RequestResource(m_Device, m_Recorder, m_Mutexes.compute_pipeline, m_State.compute_pipelines, m_PipelineCache, pipeline_state);
    }

    DescriptorSet &ResourceCache::RequestDescriptorSet(DescriptorSetLayout &descriptor_set_layout, const BindingMap<VkDescriptorBufferInfo> &buffer_infos, const BindingMap<VkDescriptorImageInfo> &image_infos)
    {
        auto &descriptor_pool = RequestResource(m_Device, m_Recorder, m_Mutexes.descriptor_set, m_State.descriptor_pools, descriptor_set_layout);
        return RequestResource(m_Device, m_Recorder, m_Mutexes.descriptor_set, m_State.descriptor_sets, descriptor_set_layout, descriptor_pool, buffer_infos, image_infos);
    }

    RenderPass &ResourceCache::RequestRenderPass(const std::vector<Attachment> &attachments, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<SubpassInfo> &subpasses)
    {
        return RequestResource(m_Device, m_Recorder, m_Mutexes.render_pass, m_State.render_passes, attachments, load_store_infos, subpasses);
    }

    Framebuffer &ResourceCache::RequestFramebuffer(const RenderTarget &render_target, const RenderPass &render_pass)
    {
        return RequestResource(m_Device, m_Recorder, m_Mutexes.framebuffer, m_State.framebuffers, render_target, render_pass);
    }

    void ResourceCache::ClearPipelines()
    {
        m_State.graphics_pipelines.clear();
        m_State.compute_pipelines.clear();
    }

    void ResourceCache::UpdateDescriptorSets(const std::vector<core::ImageView> &old_views, const std::vector<core::ImageView> &new_views)
    {
        std::vector<VkWriteDescriptorSet> set_updates;
        std::set<size_t> matches;

        for (size_t i = 0; i < old_views.size(); ++i)
        {
            auto &old_view = old_views[i];
            auto &new_view = new_views[i];

            for (auto &kd_pair : m_State.descriptor_sets)
            {
                auto &key = kd_pair.first;
                auto &descriptor_set = kd_pair.second;

                auto &image_infos = descriptor_set.GetImageInfos();

                for (auto &ba_pair : image_infos)
                {
                    auto &binding = ba_pair.first;
                    auto &array = ba_pair.second;

                    for (auto &ai_pair : array)
                    {
                        auto &array_element = ai_pair.first;
                        auto &image_info = ai_pair.second;

                        if (image_info.imageView == old_view.GetHandle())
                        {
                            // Save key to remove old descriptor set
                            matches.insert(key);

                            // Update image info with new view
                            image_info.imageView = new_view.GetHandle();

                            // Save struct for writing the update later
                            {
                                VkWriteDescriptorSet write_descriptor_set{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

                                if (auto binding_info = descriptor_set.GetLayout().GetLayoutBinding(binding))
                                {
                                    write_descriptor_set.dstBinding = binding;
                                    write_descriptor_set.descriptorType = binding_info->descriptorType;
                                    write_descriptor_set.pImageInfo = &image_info;
                                    write_descriptor_set.dstSet = descriptor_set.GetHandle();
                                    write_descriptor_set.dstArrayElement = array_element;
                                    write_descriptor_set.descriptorCount = 1;

                                    set_updates.push_back(write_descriptor_set);
                                }
                                else
                                {
                                    ENG_CORE_ERROR("Shader layout set does not use image binding at #{}", binding);
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!set_updates.empty())
        {
            vkUpdateDescriptorSets(m_Device.GetHandle(), ToUint32_t(set_updates.size()), set_updates.data(),
                                   0, nullptr);
        }

        // Delete old entries (moved out descriptor sets)
        for (auto &match : matches)
        {
            // Move out of the map
            auto it = m_State.descriptor_sets.find(match);
            auto descriptor_set = std::move(it->second);
            m_State.descriptor_sets.erase(match);

            // Generate new key
            size_t new_key = 0U;
            HashParam(new_key, descriptor_set.GetLayout(), descriptor_set.GetBufferInfos(), descriptor_set.GetImageInfos());

            // Add (key, resource) to the cache
            m_State.descriptor_sets.emplace(new_key, std::move(descriptor_set));
        }
    }

    void ResourceCache::ClearFramebuffers()
    {
        m_State.framebuffers.clear();
    }

    void ResourceCache::Clear()
    {
        m_State.shader_modules.clear();
        m_State.pipeline_layouts.clear();
        m_State.descriptor_sets.clear();
        m_State.descriptor_set_layouts.clear();
        m_State.render_passes.clear();
        ClearPipelines();
        ClearFramebuffers();
    }
}
