#include "vulkan_api/core/descriptor_pool.h"

#include "vulkan_api/core/descriptor_set_layout.h"

namespace engine
{
    DescriptorPool::DescriptorPool(Device &device,
                                   const DescriptorSetLayout &descriptor_set_layout,
                                   uint32_t pool_size)
        : m_Device(device),
          m_DescriptorSetLayout(&descriptor_set_layout)
    {
        const auto &bindings = descriptor_set_layout.GetBindings();
        std::map<VkDescriptorType, std::uint32_t> descriptor_type_counts;

        for (auto &binding : bindings)
            descriptor_type_counts[binding.descriptorType] += binding.descriptorCount;

        m_PoolSizes.resize(descriptor_type_counts.size());
        auto pool_size_it = m_PoolSizes.begin();

        for (auto &it : descriptor_type_counts)
        {
            pool_size_it->type = it.first;
            pool_size_it->descriptorCount = it.second * pool_size;
            ++pool_size_it;
        }

        m_PoolMaxSets = pool_size;
    }

    DescriptorPool::~DescriptorPool()
    {
    }

    const DescriptorSetLayout &DescriptorPool::GetDescriptorSetLayout() const
    {
        assert(m_DescriptorSetLayout && "Descriptor set layout is invalid");
        return *m_DescriptorSetLayout;
    }
}
