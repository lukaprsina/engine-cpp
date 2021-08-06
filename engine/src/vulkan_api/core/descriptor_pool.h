#pragma once

namespace engine
{
    class Device;
    class DescriptorSetLayout;

    class DescriptorPool
    {
    public:
        static const uint32_t MAX_SETS_PER_POOL = 16;
        DescriptorPool(Device &device,
                       const DescriptorSetLayout &descriptor_set_layout,
                       uint32_t pool_size = MAX_SETS_PER_POOL);
        ~DescriptorPool();

        const DescriptorSetLayout &GetDescriptorSetLayout() const;

    private:
        Device &m_Device;
        const DescriptorSetLayout *m_DescriptorSetLayout{nullptr};
        std::vector<VkDescriptorPoolSize> m_PoolSizes;
        uint32_t m_PoolMaxSets{0};
        std::vector<VkDescriptorPool> m_Pools;
        std::vector<uint32_t> m_PoolSetsCount;
        uint32_t m_PoolIndex{0};
        std::unordered_map<VkDescriptorSet, uint32_t> m_SetPoolMapping;

        uint32_t FindAvailablePool(uint32_t pool_index);
    };
}
