#include "vulkan_api/render_frame.h"

#include "vulkan_api/device.h"
#include "vulkan_api/queue_family.h"
#include "vulkan_api/core/descriptor_set.h"
#include "vulkan_api/core/descriptor_pool.h"
#include "common/resource_caching.h"

namespace engine
{
    namespace
    {
        const std::string BufferUsageToString(VkBufferUsageFlags flags)
        {
            return ToString<VkBufferUsageFlagBits>(flags,
                                                   {{VK_BUFFER_USAGE_TRANSFER_SRC_BIT, "VK_BUFFER_USAGE_TRANSFER_SRC_BIT"},
                                                    {VK_BUFFER_USAGE_TRANSFER_DST_BIT, "VK_BUFFER_USAGE_TRANSFER_DST_BIT"},
                                                    {VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, "VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT"},
                                                    {VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, "VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT"},
                                                    {VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, "VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT"},
                                                    {VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, "VK_BUFFER_USAGE_STORAGE_BUFFER_BIT"},
                                                    {VK_BUFFER_USAGE_INDEX_BUFFER_BIT, "VK_BUFFER_USAGE_INDEX_BUFFER_BIT"},
                                                    {VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, "VK_BUFFER_USAGE_VERTEX_BUFFER_BIT"},
                                                    {VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT"},
                                                    {VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, "VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT"},
                                                    {VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT, "VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT"},
                                                    {VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT, "VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT"},
                                                    {VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT, "VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT"},
                                                    {VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, "VK_BUFFER_USAGE_RAY_TRACING_BIT_NV"},
                                                    {VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT, "VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT"},
                                                    {VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR, "VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR"}});
        }
    }
    RenderFrame::RenderFrame(Device &device,
                             std::unique_ptr<RenderTarget> &&render_target,
                             size_t thread_count)
        : m_Device(device),
          m_FencePool(device),
          m_SemaphorePool(device),
          m_SwapchainRenderTarget(std::move(render_target)),
          m_ThreadCount(thread_count)
    {
        for (auto &usage_it : supported_usage_map)
        {
            std::vector<std::pair<BufferPool, BufferBlock *>> usage_buffer_pools;
            for (size_t i = 0; i < thread_count; ++i)
            {
                usage_buffer_pools.push_back(std::make_pair(BufferPool{device, BUFFER_POOL_BLOCK_SIZE * 1024 * usage_it.second, usage_it.first}, nullptr));
            }

            auto res_ins_it = m_BufferPools.emplace(usage_it.first, std::move(usage_buffer_pools));

            if (!res_ins_it.second)
            {
                throw std::runtime_error("Failed to insert buffer pool");
            }
        }

        for (size_t i = 0; i < thread_count; ++i)
        {
            m_DescriptorPools.push_back(std::make_unique<std::unordered_map<std::size_t, DescriptorPool>>());
            m_DescriptorSets.push_back(std::make_unique<std::unordered_map<std::size_t, DescriptorSet>>());
        }
    }

    RenderFrame::~RenderFrame()
    {
    }

    void RenderFrame::Reset()
    {
        VK_CHECK(m_FencePool.Wait());

        m_FencePool.Reset();

        for (auto &command_pools_per_queue : m_CommandPools)
        {
            for (auto &command_pool : command_pools_per_queue.second)
            {
                command_pool->ResetPool();
            }
        }

        for (auto &buffer_pools_per_usage : m_BufferPools)
        {
            for (auto &buffer_pool : buffer_pools_per_usage.second)
            {
                buffer_pool.first.Reset();
                buffer_pool.second = nullptr;
            }
        }

        m_SemaphorePool.Reset();
    }

    BufferAllocation RenderFrame::AllocateBuffer(VkBufferUsageFlags usage, VkDeviceSize size, size_t thread_index)
    {
        ENG_ASSERT(thread_index < m_ThreadCount, "Thread index is out of bounds");

        uint32_t block_multiplier = supported_usage_map.at(usage);

        if (size > BUFFER_POOL_BLOCK_SIZE * 1024 * block_multiplier)
        {
            ENG_CORE_ERROR("Trying to allocate {} buffer of size {}KB which is larger than the buffer pool block size ({} KB)!", BufferUsageToString(usage), size / 1024, BUFFER_POOL_BLOCK_SIZE * block_multiplier);
            throw std::runtime_error("Couldn't allocate render frame buffer.");
        }

        // Find a pool for this usage
        auto buffer_pool_it = m_BufferPools.find(usage);
        if (buffer_pool_it == m_BufferPools.end())
        {
            ENG_CORE_ERROR("No buffer pool for buffer usage {}", usage);
            return BufferAllocation{};
        }

        auto &buffer_pool = buffer_pool_it->second.at(thread_index).first;
        auto &buffer_block = buffer_pool_it->second.at(thread_index).second;

        if (m_BufferAllocationStrategy == BufferAllocationStrategy::OneAllocationPerBuffer || !buffer_block)
        {
            // If there is no block associated with the pool or we are creating a buffer for each allocation,
            // request a new buffer block
            buffer_block = &buffer_pool.RequestBufferBlock(ToUint32_t(size));
        }

        auto data = buffer_block->Allocate(ToUint32_t(size));

        // Check if the buffer block can allocate the requested size
        if (data.Empty())
        {
            buffer_block = &buffer_pool.RequestBufferBlock(ToUint32_t(size));

            data = buffer_block->Allocate(ToUint32_t(size));
        }

        return data;
    }

    std::vector<std::unique_ptr<CommandPool>> &RenderFrame::GetCommandPools(const QueueFamily &queue_family,
                                                                            CommandBuffer::ResetMode reset_mode)

    {
        auto command_pool_it = m_CommandPools.find(queue_family.GetFamilyIndex());

        if (command_pool_it != m_CommandPools.end())
        {
            if (command_pool_it->second.at(0)->GetResetMode() != reset_mode)
            {
                m_Device.WaitIdle();

                // Delete pools
                m_CommandPools.erase(command_pool_it);
            }
            else
            {
                return command_pool_it->second;
            }
        }

        std::vector<std::unique_ptr<CommandPool>> queue_command_pools;
        for (size_t i = 0; i < m_ThreadCount; i++)
        {
            queue_command_pools.push_back(std::make_unique<CommandPool>(m_Device, queue_family.GetFamilyIndex(),
                                                                        this, i, reset_mode));
        }

        auto res_ins_it = m_CommandPools.emplace(queue_family.GetFamilyIndex(), std::move(queue_command_pools));

        if (!res_ins_it.second)
        {
            throw std::runtime_error("Failed to insert command pool");
        }

        command_pool_it = res_ins_it.first;

        return command_pool_it->second;
    }

    DescriptorSet &RenderFrame::RequestDescriptorSet(DescriptorSetLayout &descriptor_set_layout, const BindingMap<VkDescriptorBufferInfo> &buffer_infos, const BindingMap<VkDescriptorImageInfo> &image_infos, size_t thread_index)
    {
        ENG_ASSERT(thread_index < m_ThreadCount, "Thread index is out of bounds");
        // TODO: request resource
        auto &descriptor_pool = RequestResource(m_Device, nullptr, *m_DescriptorPools.at(thread_index), descriptor_set_layout);
        return RequestResource(m_Device, nullptr, *m_DescriptorSets.at(thread_index), descriptor_set_layout, descriptor_pool, buffer_infos, image_infos);
    }

    CommandBuffer &RenderFrame::RequestCommandBuffer(const QueueFamily &queue_family,
                                                     CommandBuffer::ResetMode reset_mode,
                                                     VkCommandBufferLevel level, size_t thread_index)
    {
        assert(thread_index < m_ThreadCount && "Thread index is out of bounds");

        auto &command_pools = GetCommandPools(queue_family, reset_mode);

        auto command_pool_it = std::find_if(command_pools.begin(), command_pools.end(), [&thread_index](std::unique_ptr<CommandPool> &cmd_pool)
                                            { return cmd_pool->GetThreadIndex() == thread_index; });

        return (*command_pool_it)->RequestCommandBuffer(level);
    }
}
