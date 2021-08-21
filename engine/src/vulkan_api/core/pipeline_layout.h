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

        VkPipelineLayout GetHandle() const { return m_Handle; }
        const std::vector<ShaderModule *> &GetShaderModules() const { return m_ShaderModules; }
        const std::vector<ShaderResource> GetResources(const ShaderResourceType &type = ShaderResourceType::All, VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL) const;
        VkShaderStageFlags GetPushConstantRangeStage(uint32_t size, uint32_t offset = 0) const;

    private:
        Device &m_Device;
        VkPipelineLayout m_Handle{VK_NULL_HANDLE};
        std::vector<ShaderModule *> m_ShaderModules;
        std::unordered_map<std::string, ShaderResource> m_ShaderResources;
    };
}
