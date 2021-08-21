#include "vulkan_api/core/pipeline_layout.h"

#include "vulkan_api/core/descriptor_set_layout.h"
#include "vulkan_api/device.h"

namespace engine
{
    PipelineLayout::PipelineLayout(Device &device,
                                   const std::vector<ShaderModule *> &shader_modules)
        : m_Device(device), m_ShaderModules(shader_modules)
    {
        for (auto *shader_module : shader_modules)
        {
            for (const auto &shader_resource : shader_module->GetResources())
            {
                std::string key = shader_resource.name;

                // Since 'Input' and 'Output' resources can have the same name, we modify the key string
                if (shader_resource.type == ShaderResourceType::Input || shader_resource.type == ShaderResourceType::Output)
                {
                    key = std::to_string(shader_resource.stages) + "_" + key;
                }

                auto it = m_ShaderResources.find(key);

                if (it != m_ShaderResources.end())
                {
                    // Append stage flags if resource already exists
                    it->second.stages |= shader_resource.stages;
                }
                else
                {
                    // Create a new entry in the map
                    m_ShaderResources.emplace(key, shader_resource);
                }
            }
        }

        // Sift through the map of name indexed shader resources
        // Separate them into their respective sets
        for (auto &it : m_ShaderResources)
        {
            auto &shader_resource = it.second;

            // Find binding by set index in the map.
            auto it2 = m_ShaderSets.find(shader_resource.set);

            if (it2 != m_ShaderSets.end())
            {
                // Add resource to the found set index
                it2->second.push_back(shader_resource);
            }
            else
            {
                // Create a new set index and with the first resource
                m_ShaderSets.emplace(shader_resource.set, std::vector<ShaderResource>{shader_resource});
            }
        }

        // Create a descriptor set layout for each shader set in the shader modules
        for (auto &shader_set_it : m_ShaderSets)
        {
            m_DescriptorSetLayouts.emplace_back(&m_Device.GetResourceCache().RequestDescriptorSetLayout(shader_set_it.first, shader_modules, shader_set_it.second));
        }

        // Collect all the descriptor set layout handles, maintaining set order
        std::vector<VkDescriptorSetLayout> descriptor_set_layout_handles;
        for (uint32_t i = 0; i < m_DescriptorSetLayouts.size(); ++i)
        {
            if (m_DescriptorSetLayouts[i])
            {
                descriptor_set_layout_handles.push_back(m_DescriptorSetLayouts[i]->GetHandle());
            }
            else
            {
                descriptor_set_layout_handles.push_back(VK_NULL_HANDLE);
            }
        }

        // Collect all the push constant shader resources
        std::vector<VkPushConstantRange> push_constant_ranges;
        for (auto &push_constant_resource : GetResources(ShaderResourceType::PushConstant))
        {
            push_constant_ranges.push_back({push_constant_resource.stages, push_constant_resource.offset, push_constant_resource.size});
        }

        VkPipelineLayoutCreateInfo create_info{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

        create_info.setLayoutCount = ToUint32_t(descriptor_set_layout_handles.size());
        create_info.pSetLayouts = descriptor_set_layout_handles.data();
        create_info.pushConstantRangeCount = ToUint32_t(push_constant_ranges.size());
        create_info.pPushConstantRanges = push_constant_ranges.data();

        // Create the Vulkan pipeline layout handle
        auto result = vkCreatePipelineLayout(device.GetHandle(), &create_info, nullptr, &m_Handle);

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

    DescriptorSetLayout &PipelineLayout::GetDescriptorSetLayout(const uint32_t set_index) const
    {
        for (auto &descriptor_set_layout : m_DescriptorSetLayouts)
        {
            if (descriptor_set_layout->GetIndex() == set_index)
            {
                return *descriptor_set_layout;
            }
        }
        throw std::runtime_error("Couldn't find descriptor set layout at set index " + ToString(set_index));
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
