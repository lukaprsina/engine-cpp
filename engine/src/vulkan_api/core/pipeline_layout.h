#pragma once

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

    private:
        Device &m_Device;
        VkPipelineLayout m_Handle{VK_NULL_HANDLE};
        std::vector<ShaderModule *> m_ShaderModules;
    };
}
