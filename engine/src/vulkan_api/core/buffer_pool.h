#pragma once

#include "vulkan_api/core/buffer.h"

namespace engine
{
    class BufferAllocation
    {
    public:
        BufferAllocation() = default;
        BufferAllocation(core::Buffer &buffer, VkDeviceSize size, VkDeviceSize offset);
        BufferAllocation(const BufferAllocation &) = delete;
        BufferAllocation(BufferAllocation &&) = default;
        BufferAllocation &operator=(const BufferAllocation &) = delete;
        BufferAllocation &operator=(BufferAllocation &&) = default;

        void Update(const std::vector<uint8_t> &data, uint32_t offset = 0);

        bool Empty() const
        {
            return m_Size == 0 || m_Buffer == nullptr;
        }

        template <class T>
        void Update(const T &value, uint32_t offset = 0)
        {
            Update(ToBytes(value), offset);
        }

        core::Buffer &GetBuffer();
        VkDeviceSize GetSize() const { return m_Size; }
        VkDeviceSize GetOffset() const { return m_BaseOffset; }

    private:
        core::Buffer *m_Buffer{nullptr};
        VkDeviceSize m_Size{0};
        VkDeviceSize m_BaseOffset{0};
    };

    class BufferBlock
    {
    public:
        BufferBlock(Device &device, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);

        BufferAllocation Allocate(uint32_t size);
        void Reset() { m_Offset = 0; }

        VkDeviceSize GetSize() const { return m_Buffer.GetSize(); }

    private:
        core::Buffer m_Buffer;
        VkDeviceSize m_Alignment{0};
        VkDeviceSize m_Offset{0};
    };

    class BufferPool
    {
    public:
        BufferPool(Device &device, VkDeviceSize block_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU);

        BufferBlock &RequestBufferBlock(VkDeviceSize minimum_size);
        void Reset();

    private:
        Device &m_Device;
        std::vector<std::unique_ptr<BufferBlock>> m_BufferBlocks;
        VkDeviceSize m_BlockSize{0};
        VkBufferUsageFlags m_Usage{};
        VmaMemoryUsage m_MemoryUsage{};
        uint32_t m_ActiveBufferBlockCount{0};
    };
}
