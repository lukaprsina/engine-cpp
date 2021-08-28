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
        for (auto pool : m_Pools)
        {
            vkDestroyDescriptorPool(m_Device.GetHandle(), pool, nullptr);
        }
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

    std::uint32_t DescriptorPool::FindAvailablePool(std::uint32_t search_index)
    {
        // Create a new pool
        if (m_Pools.size() <= search_index)
        {
            VkDescriptorPoolCreateInfo create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};

            create_info.poolSizeCount = ToUint32_t(m_PoolSizes.size());
            create_info.pPoolSizes = m_PoolSizes.data();
            create_info.maxSets = m_PoolMaxSets;

            // We do not set FREE_DESCRIPTOR_SET_BIT as we do not need to free individual descriptor sets
            create_info.flags = 0;

            // Check descriptor set layout and enable the required flags
            auto &binding_flags = m_DescriptorSetLayout->GetBindingFlags();
            for (auto binding_flag : binding_flags)
            {
                if (binding_flag & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT)
                {
                    create_info.flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
                }
            }

            VkDescriptorPool handle = VK_NULL_HANDLE;

            // Create the Vulkan descriptor pool
            auto result = vkCreateDescriptorPool(m_Device.GetHandle(), &create_info, nullptr, &handle);

            if (result != VK_SUCCESS)
            {
                return 0;
            }

            // Store internally the Vulkan handle
            m_Pools.push_back(handle);

            // Add set count for the descriptor pool
            m_PoolSetsCount.push_back(0);

            return search_index;
        }
        else if (m_PoolSetsCount[search_index] < m_PoolMaxSets)
        {
            return search_index;
        }

        // Increment pool index
        return FindAvailablePool(++search_index);
    }
}
