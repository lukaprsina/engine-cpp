#pragma once

namespace engine
{
    class DescriptorPool;
    class Device;
    class ShaderModule;
    struct ShaderResource;

    class DescriptorSetLayout
    {
    public:
        DescriptorSetLayout(Device &device,
                            const uint32_t set_index,
                            const std::vector<ShaderModule *> &shader_modules,
                            const std::vector<ShaderResource> &resource_set);
        ~DescriptorSetLayout();
        DescriptorSetLayout(DescriptorSetLayout &&other);

        VkDescriptorSetLayout GetHandle() const { return m_Handle; }
        const std::vector<VkDescriptorSetLayoutBinding> &GetBindings() const { return m_Bindings; }
        const std::vector<VkDescriptorBindingFlagsEXT> &GetBindingFlags() const { return m_BindingFlags; }
        std::unique_ptr<VkDescriptorSetLayoutBinding> GetLayoutBinding(uint32_t binding_index) const;
        const uint32_t GetIndex() const { return m_SetIndex; }

    private:
        Device &m_Device;
        VkDescriptorSetLayout m_Handle{VK_NULL_HANDLE};
        const uint32_t m_SetIndex;
        std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
        std::vector<VkDescriptorBindingFlagsEXT> m_BindingFlags;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_BindingsLookup;
        std::unordered_map<uint32_t, VkDescriptorBindingFlagsEXT> m_BindingFlagsLookup;
        std::unordered_map<std::string, uint32_t> m_ResourcesLookup;
        std::vector<ShaderModule *> m_ShaderModules;
    };
}
