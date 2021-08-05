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

            void Unmap();

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
