#include "vulkan_api/core/pipeline_layout.h"

namespace engine
{
    PipelineLayout::PipelineLayout(Device &device,
                                   const std::vector<ShaderModule *> &shader_modules)
        : m_Device(device), m_ShaderModules(shader_modules)
    {
        for (auto *shader_module : shader_modules)
        {
            for (const auto &shader_resource : shader_module->get_resources())
            {
                std::string key = shader_resource.name;

                // Since 'Input' and 'Output' resources can have the same name, we modify the key string
                if (shader_resource.type == ShaderResourceType::Input || shader_resource.type == ShaderResourceType::Output)
                {
                    key = std::to_string(shader_resource.stages) + "_" + key;
                }

                auto it = shader_resources.find(key);

                if (it != shader_resources.end())
                {
                    // Append stage flags if resource already exists
                    it->second.stages |= shader_resource.stages;
                }
                else
                {
                    // Create a new entry in the map
                    shader_resources.emplace(key, shader_resource);
                }
            }
        }

        // Sift through the map of name indexed shader resources
        // Separate them into their respective sets
        for (auto &it : shader_resources)
        {
            auto &shader_resource = it.second;

            // Find binding by set index in the map.
            auto it2 = shader_sets.find(shader_resource.set);

            if (it2 != shader_sets.end())
            {
                // Add resource to the found set index
                it2->second.push_back(shader_resource);
            }
            else
            {
                // Create a new set index and with the first resource
                shader_sets.emplace(shader_resource.set, std::vector<ShaderResource>{shader_resource});
            }
        }

        // Create a descriptor set layout for each shader set in the shader modules
        for (auto &shader_set_it : shader_sets)
        {
            descriptor_set_layouts.emplace_back(&device.get_resource_cache().request_descriptor_set_layout(shader_set_it.first, shader_modules, shader_set_it.second));
        }

        // Collect all the descriptor set layout handles, maintaining set order
        std::vector<VkDescriptorSetLayout> descriptor_set_layout_handles;
        for (uint32_t i = 0; i < descriptor_set_layouts.size(); ++i)
        {
            if (descriptor_set_layouts[i])
            {
                descriptor_set_layout_handles.push_back(descriptor_set_layouts[i]->get_handle());
            }
            else
            {
                descriptor_set_layout_handles.push_back(VK_NULL_HANDLE);
            }
        }

        // Collect all the push constant shader resources
        std::vector<VkPushConstantRange> push_constant_ranges;
        for (auto &push_constant_resource : get_resources(ShaderResourceType::PushConstant))
        {
            push_constant_ranges.push_back({push_constant_resource.stages, push_constant_resource.offset, push_constant_resource.size});
        }

        VkPipelineLayoutCreateInfo create_info{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

        create_info.setLayoutCount = to_u32(descriptor_set_layout_handles.size());
        create_info.pSetLayouts = descriptor_set_layout_handles.data();
        create_info.pushConstantRangeCount = to_u32(push_constant_ranges.size());
        create_info.pPushConstantRanges = push_constant_ranges.data();

        // Create the Vulkan pipeline layout handle
        auto result = vkCreatePipelineLayout(device.get_handle(), &create_info, nullptr, &handle);

        if (result != VK_SUCCESS)
        {
            throw VulkanException{result, "Cannot create PipelineLayout"};
        }
    }

    PipelineLayout::~PipelineLayout()
    {
    }

    const std::vector<ShaderResource> PipelineLayout::GetResources(const ShaderResourceType &type, VkShaderStageFlagBits stage) const
    {
        std::vector<ShaderResource> found_resources;

        for (auto &it : m_ShaderResources)
        {
            auto &shader_resource = it.second;

            if (shader_resource.type == type || type == ShaderResourceType::All)
            {
                if (shader_resource.stages == stage || stage == VK_SHADER_STAGE_ALL)
                {
                    found_resources.push_back(shader_resource);
                }
            }
        }

        return found_resources;
    }

    VkShaderStageFlags PipelineLayout::GetPushConstantRangeStage(uint32_t size, uint32_t offset) const
    {
        VkShaderStageFlags stages = 0;

        for (auto &push_constant_resource : GetResources(ShaderResourceType::PushConstant))
        {
            if (offset >= push_constant_resource.offset && offset + size <= push_constant_resource.offset + push_constant_resource.size)
            {
                stages |= push_constant_resource.stages;
            }
        }
        return stages;
    }
}
