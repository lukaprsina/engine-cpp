#include "vulkan_api/core/descriptor_pool.h"

#include "vulkan_api/core/descriptor_set_layout.h"
#include "vulkan_api/device.h"

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

    VkDescriptorSet DescriptorPool::Allocate()
    {
        m_PoolIndex = FindAvailablePool(m_PoolIndex);

        // Increment allocated set count for the current pool
        ++m_PoolSetsCount[m_PoolIndex];

        VkDescriptorSetLayout set_layout = GetDescriptorSetLayout().GetHandle();

        VkDescriptorSetAllocateInfo alloc_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        alloc_info.descriptorPool = m_Pools[m_PoolIndex];
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &set_layout;

        VkDescriptorSet handle = VK_NULL_HANDLE;

        // Allocate a new descriptor set from the current pool
        auto result = vkAllocateDescriptorSets(m_Device.GetHandle(), &alloc_info, &handle);

        if (result != VK_SUCCESS)
        {
            // Decrement allocated set count for the current pool
            --m_PoolSetsCount[m_PoolIndex];

            return VK_NULL_HANDLE;
        }

        // Store mapping between the descriptor set and the pool
        m_SetPoolMapping.emplace(handle, m_PoolIndex);

        return handle;
    }

    const DescriptorSetLayout &DescriptorPool::GetDescriptorSetLayout() const
    {
        assert(m_DescriptorSetLayout && "Descriptor set layout is invalid");
        return *m_DescriptorSetLayout;
    }
}
