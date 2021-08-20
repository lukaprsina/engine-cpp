#include "vulkan_api/core/buffer_pool.h"

#include "vulkan_api/device.h"

namespace engine
{
    BufferAllocation::BufferAllocation(core::Buffer &buffer, VkDeviceSize size, VkDeviceSize offset)
        : m_Buffer(&buffer),
          m_Size(size),
          m_BaseOffset(offset)
    {
    }

    BufferAllocation::~BufferAllocation()
    {
    }

    void BufferAllocation::Update(const std::vector<uint8_t> &data, uint32_t offset)
    {
        ENG_ASSERT(m_Buffer, "Invalid buffer pointer");

        if (offset + data.size() <= m_Size)
            m_Buffer->Update(data, ToUint32_t(m_BaseOffset) + offset);
        else
            ENG_CORE_WARN("Ignore buffer allocation update");
    }

    BufferBlock::BufferBlock(Device &device, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage)
        : m_Buffer{device, size, usage, memory_usage}

    {
        if (usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
            m_Alignment = device.GetGPU().GetProperties().limits.minUniformBufferOffsetAlignment;

        else if (usage == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
            m_Alignment = device.GetGPU().GetProperties().limits.minStorageBufferOffsetAlignment;

        else if (usage == VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)
            m_Alignment = device.GetGPU().GetProperties().limits.minTexelBufferOffsetAlignment;

        else if (usage == VK_BUFFER_USAGE_INDEX_BUFFER_BIT || usage == VK_BUFFER_USAGE_VERTEX_BUFFER_BIT || usage == VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)
            m_Alignment = 16;

        else
            throw std::runtime_error("Usage not recognised");
    }

    BufferBlock::~BufferBlock()
    {
    }

    BufferAllocation BufferBlock::Allocate(const uint32_t allocate_size)
    {
        ENG_ASSERT(allocate_size > 0, "Allocation size must be greater than zero");

        auto aligned_offset = (m_Offset + m_Alignment - 1) & ~(m_Alignment - 1);

        if (aligned_offset + allocate_size > m_Buffer.GetSize())
            return BufferAllocation{};

        m_Offset = aligned_offset + allocate_size;
        return BufferAllocation{m_Buffer, allocate_size, aligned_offset};
    }

    BufferPool::BufferPool(Device &device, VkDeviceSize block_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage)
        : m_Device{device},
          m_BlockSize{block_size},
          m_Usage{usage},
          m_MemoryUsage{memory_usage}
    {
    }

    BufferPool::~BufferPool()
    {
    }

    BufferBlock &BufferPool::RequestBufferBlock(VkDeviceSize minimum_size)
    {
        auto it = std::upper_bound(m_BufferBlocks.begin() + m_ActiveBufferBlockCount, m_BufferBlocks.end(), minimum_size,
                                   [](const VkDeviceSize &a, const std::unique_ptr<BufferBlock> &b) -> bool
                                   { return a <= b->GetSize(); });

        if (it != m_BufferBlocks.end())
        {
            // Recycle inactive block
            m_ActiveBufferBlockCount++;
            return *it->get();
        }

        ENG_CORE_TRACE("Building #{} buffer block ({})", m_BufferBlocks.size(), m_Usage);

        // Create a new block, store and return it
        m_BufferBlocks.emplace_back(std::make_unique<BufferBlock>(m_Device, std::max(m_BlockSize, minimum_size), m_Usage, m_MemoryUsage));

        auto &block = m_BufferBlocks[m_ActiveBufferBlockCount++];

        return *block.get();
    }

    void BufferPool::Reset()
    {
        for (auto &buffer_block : m_BufferBlocks)
        {
            buffer_block->Reset();
        }

        m_ActiveBufferBlockCount = 0;
    }
}
