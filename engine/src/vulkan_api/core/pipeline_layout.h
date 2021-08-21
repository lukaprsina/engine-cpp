#pragma once

#include "renderer/shader.h"

namespace engine
{
    class Device;
    class ShaderModule;
    class DescriptorSetLayout;

    class PipelineLayout
    {
    public:
        PipelineLayout(Device &device, const std::vector<ShaderModule *> &shader_modules);
        ~PipelineLayout();

        bool HasDescriptorSetLayout(const uint32_t set_index) const
        {
            return set_index < m_DescriptorSetLayouts.size();
        }

        VkPipelineLayout GetHandle() const { return m_Handle; }
        const std::vector<ShaderModule *> &GetShaderModules() const { return m_ShaderModules; }
        const std::vector<ShaderResource> GetResources(const ShaderResourceType &type = ShaderResourceType::All, VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL) const;
        DescriptorSetLayout &GetDescriptorSetLayout(const uint32_t set_index) const;
        VkShaderStageFlags GetPushConstantRangeStage(uint32_t size, uint32_t offset = 0) const;
        const std::unordered_map<uint32_t, std::vector<ShaderResource>> &GetShaderSets() const { return m_ShaderSets; }

    private:
        Device &m_Device;
        VkPipelineLayout m_Handle{VK_NULL_HANDLE};
        std::vector<ShaderModule *> m_ShaderModules;
        std::unordered_map<std::string, ShaderResource> m_ShaderResources;
        std::unordered_map<uint32_t, std::vector<ShaderResource>> m_ShaderSets;
        std::vector<DescriptorSetLayout *> m_DescriptorSetLayouts;
    };
}
