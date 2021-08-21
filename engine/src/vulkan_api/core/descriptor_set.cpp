#include "vulkan_api/core/descriptor_set.h"

#include "vulkan_api/device.h"
#include "common/resource_caching.h"
#include "vulkan_api/core/descriptor_set.h"
#include "vulkan_api/core/descriptor_set_layout.h"

namespace engine
{
    DescriptorSet::DescriptorSet(Device &device,
                                 DescriptorSetLayout &m_DescriptorSetLayout,
                                 DescriptorPool &descriptor_pool,
                                 const BindingMap<VkDescriptorBufferInfo> &buffer_infos,
                                 const BindingMap<VkDescriptorImageInfo> &image_infos)
        : m_Device(device), m_DescriptorSetLayout(m_DescriptorSetLayout),
          m_DescriptorPool(descriptor_pool), m_BufferInfos(buffer_infos),
          m_ImageInfos(image_infos), m_Handle(descriptor_pool.Allocate())
    {
        Prepare();
    }

    DescriptorSet::DescriptorSet(DescriptorSet &&other)
        : m_Device{other.m_Device},
          m_DescriptorSetLayout{other.m_DescriptorSetLayout},
          m_DescriptorPool{other.m_DescriptorPool},
          m_BufferInfos{std::move(other.m_BufferInfos)},
          m_ImageInfos{std::move(other.m_ImageInfos)},
          m_Handle{other.m_Handle},
          m_WriteDescriptorSets{std::move(other.m_WriteDescriptorSets)},
          m_UpdatedBindings{std::move(other.m_UpdatedBindings)}
    {
        other.m_Handle = VK_NULL_HANDLE;
    }

    void DescriptorSet::Prepare()
    {
        if (!m_WriteDescriptorSets.empty())
        {
            ENG_CORE_WARN("Trying to prepare a descriptor set that has already been prepared, skipping.");
            return;
        }

        // Iterate over all buffer bindings
        for (auto &binding_it : m_BufferInfos)
        {
            auto binding_index = binding_it.first;
            auto &buffer_bindings = binding_it.second;

            if (auto binding_info = m_DescriptorSetLayout.GetLayoutBinding(binding_index))
            {
                // Iterate over all binding buffers in array
                for (auto &element_it : buffer_bindings)
                {
                    auto &buffer_info = element_it.second;

                    size_t uniform_buffer_range_limit = m_Device.GetGPU().GetProperties().limits.maxUniformBufferRange;
                    size_t storage_buffer_range_limit = m_Device.GetGPU().GetProperties().limits.maxStorageBufferRange;

                    size_t buffer_range_limit = static_cast<size_t>(buffer_info.range);

                    if ((binding_info->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || binding_info->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) && buffer_range_limit > uniform_buffer_range_limit)
                    {
                        ENG_CORE_ERROR("Set {} binding {} cannot be updated: buffer size {} exceeds the uniform buffer range limit {}", m_DescriptorSetLayout.GetIndex(), binding_index, buffer_info.range, uniform_buffer_range_limit);
                        buffer_range_limit = uniform_buffer_range_limit;
                    }
                    else if ((binding_info->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || binding_info->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) && buffer_range_limit > storage_buffer_range_limit)
                    {
                        ENG_CORE_ERROR("Set {} binding {} cannot be updated: buffer size {} exceeds the storage buffer range limit {}", m_DescriptorSetLayout.GetIndex(), binding_index, buffer_info.range, storage_buffer_range_limit);
                        buffer_range_limit = storage_buffer_range_limit;
                    }

                    // Clip the buffers range to the limit if one exists as otherwise we will receive a Vulkan validation error
                    buffer_info.range = buffer_range_limit;

                    VkWriteDescriptorSet write_descriptor_set{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

                    write_descriptor_set.dstBinding = binding_index;
                    write_descriptor_set.descriptorType = binding_info->descriptorType;
                    write_descriptor_set.pBufferInfo = &buffer_info;
                    write_descriptor_set.dstSet = m_Handle;
                    write_descriptor_set.dstArrayElement = element_it.first;
                    write_descriptor_set.descriptorCount = 1;

                    m_WriteDescriptorSets.push_back(write_descriptor_set);
                }
            }
            else
            {
                ENG_CORE_ERROR("Shader layout set does not use buffer binding at #{}", binding_index);
            }
        }

        // Iterate over all image bindings
        for (auto &binding_it : m_ImageInfos)
        {
            auto binding_index = binding_it.first;
            auto &binding_resources = binding_it.second;

            if (auto binding_info = m_DescriptorSetLayout.GetLayoutBinding(binding_index))
            {
                // Iterate over all binding images in array
                for (auto &element_it : binding_resources)
                {
                    auto &image_info = element_it.second;

                    VkWriteDescriptorSet write_descriptor_set{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

                    write_descriptor_set.dstBinding = binding_index;
                    write_descriptor_set.descriptorType = binding_info->descriptorType;
                    write_descriptor_set.pImageInfo = &image_info;
                    write_descriptor_set.dstSet = m_Handle;
                    write_descriptor_set.dstArrayElement = element_it.first;
                    write_descriptor_set.descriptorCount = 1;

                    m_WriteDescriptorSets.push_back(write_descriptor_set);
                }
            }
            else
            {
                ENG_CORE_ERROR("Shader layout set does not use image binding at #{}", binding_index);
            }
        }
    }

    void DescriptorSet::Update(const std::vector<uint32_t> &bindings_to_update)
    {
        std::vector<VkWriteDescriptorSet> write_operations;
        std::vector<size_t> write_operation_hashes;

        // If the 'bindings_to_update' vector is empty, we want to write to all the bindings
        // (but skipping all to-update bindings that haven't been written yet)
        if (bindings_to_update.empty())
        {
            for (size_t i = 0; i < m_WriteDescriptorSets.size(); i++)
            {
                const auto &write_operation = m_WriteDescriptorSets[i];

                size_t write_operation_hash = 0;
                HashParam(write_operation_hash, write_operation);

                auto update_pair_it = m_UpdatedBindings.find(write_operation.dstBinding);
                if (update_pair_it == m_UpdatedBindings.end() || update_pair_it->second != write_operation_hash)
                {
                    write_operations.push_back(write_operation);
                    write_operation_hashes.push_back(write_operation_hash);
                }
            }
        }
        else
        {
            // Otherwise we want to update the binding indices present in the 'bindings_to_update' vector.
            // (again, skipping those to update but not updated yet)
            for (size_t i = 0; i < m_WriteDescriptorSets.size(); i++)
            {
                const auto &write_operation = m_WriteDescriptorSets[i];

                if (std::find(bindings_to_update.begin(), bindings_to_update.end(), write_operation.dstBinding) != bindings_to_update.end())
                {
                    size_t write_operation_hash = 0;
                    HashParam(write_operation_hash, write_operation);

                    auto update_pair_it = m_UpdatedBindings.find(write_operation.dstBinding);
                    if (update_pair_it == m_UpdatedBindings.end() || update_pair_it->second != write_operation_hash)
                    {
                        write_operations.push_back(write_operation);
                        write_operation_hashes.push_back(write_operation_hash);
                    }
                }
            }
        }

        // Perform the Vulkan call to update the DescriptorSet by executing the write operations
        if (!write_operations.empty())
        {
            vkUpdateDescriptorSets(m_Device.GetHandle(),
                                   ToUint32_t(write_operations.size()),
                                   write_operations.data(),
                                   0,
                                   nullptr);
        }

        // Store the bindings from the write operations that were executed by vkUpdateDescriptorSets (and their hash)
        // to prevent overwriting by future calls to "update()"
        for (size_t i = 0; i < write_operations.size(); i++)
        {
            m_UpdatedBindings[write_operations[i].dstBinding] = write_operation_hashes[i];
        }
    }
}
