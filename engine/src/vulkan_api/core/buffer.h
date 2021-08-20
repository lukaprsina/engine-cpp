#pragma once

namespace engine
{
    class Device;

    namespace core
    {
        class Buffer
        {
        public:
            Buffer(Device &device, VkDeviceSize size,
                   VkBufferUsageFlags buffer_usage,
                   VmaMemoryUsage memory_usage,
                   VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_MAPPED_BIT);
            ~Buffer();

            Buffer(Buffer &&other);

            void Update(const uint8_t *data, size_t size, size_t offset = 0);
            void Update(void *data, size_t size, size_t offset = 0);
            void Update(const std::vector<uint8_t> &data, size_t offset = 0);

            uint8_t *Map();
            void Unmap();
            void Flush() const;

            VkBuffer GetHandle() const { return m_Handle; }
            VkDeviceSize GetSize() const { return m_Size; }

        private:
            Device &m_Device;
            VkBuffer m_Handle{VK_NULL_HANDLE};
            VmaAllocation m_Allocation{VK_NULL_HANDLE};
            VkDeviceMemory m_Memory{VK_NULL_HANDLE};
            VkDeviceSize m_Size{0};
            uint8_t *m_MappedData{nullptr};
            bool m_Persistent{false};
            bool m_Mapped{false};
        };
    }
}
