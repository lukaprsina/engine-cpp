#include "vulkan_api/core/buffer.h"

#include "vulkan_api/device.h"

namespace engine
{
    namespace core
    {
        Buffer::Buffer(Device &device, VkDeviceSize size,
                       VkBufferUsageFlags buffer_usage,
                       VmaMemoryUsage memory_usage,
                       VmaAllocationCreateFlags flags)
            : m_Device(device), m_Size(size)
        {
#ifdef VK_USE_PLATFORM_MACOS_MVK
            // Workaround for Mac (MoltenVK requires unmapping https://github.com/KhronosGroup/MoltenVK/issues/175)
            // Force cleares the flag VMA_ALLOCATION_CREATE_MAPPED_BIT
            flags &= ~VMA_ALLOCATION_CREATE_MAPPED_BIT;
#endif
            m_Persistent = (flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) != 0;

            VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            buffer_info.usage = buffer_usage;
            buffer_info.size = size;

            VmaAllocationCreateInfo memory_info{};
            memory_info.flags = flags;
            memory_info.usage = memory_usage;

            VmaAllocationInfo allocation_info{};
            auto result = vmaCreateBuffer(m_Device.GetMemoryAllocator(),
                                          &buffer_info, &memory_info,
                                          &m_Handle, &m_Allocation,
                                          &allocation_info);

            if (result != VK_SUCCESS)
            {
                throw VulkanException{result, "Cannot create Buffer"};
            }

            m_Memory = allocation_info.deviceMemory;

            if (m_Persistent)
            {
                m_MappedData = static_cast<uint8_t *>(allocation_info.pMappedData);
            }
        }

        Buffer::~Buffer()
        {
            if (m_Handle != VK_NULL_HANDLE && m_Allocation != VK_NULL_HANDLE)
            {
                Unmap();
                vmaDestroyBuffer(m_Device.GetMemoryAllocator(),
                                 m_Handle, m_Allocation);
            }
        }

        Buffer::Buffer(Buffer &&other)
            : m_Device{other.m_Device},
              m_Handle{other.m_Handle},
              m_Allocation{other.m_Allocation},
              m_Memory{other.m_Memory},
              m_Size{other.m_Size},
              m_MappedData{other.m_MappedData},
              m_Mapped{other.m_Mapped}
        {
            other.m_Handle = VK_NULL_HANDLE;
            other.m_Allocation = VK_NULL_HANDLE;
            other.m_Memory = VK_NULL_HANDLE;
            other.m_MappedData = nullptr;
            other.m_Mapped = false;
        }

        void Buffer::Update(const uint8_t *data, size_t size, size_t offset)
        {
            if (m_Persistent)
            {
                std::copy(data, data + size, m_MappedData + offset);
                Flush();
            }
            else
            {
                Map();
                std::copy(data, data + size, m_MappedData + offset);
                Flush();
                Unmap();
            }
        }

        void Buffer::Update(void *data, size_t size, size_t offset)
        {
            Update(reinterpret_cast<const uint8_t *>(data), size, offset);
        }

        void Buffer::Update(const std::vector<uint8_t> &data, size_t offset)
        {
            Update(data.data(), data.size(), offset);
        }

        uint8_t *Buffer::Map()
        {
            if (!m_Mapped && !m_MappedData)
            {
                VK_CHECK(vmaMapMemory(m_Device.GetMemoryAllocator(), m_Allocation, reinterpret_cast<void **>(&m_MappedData)));
                m_Mapped = true;
            }
            return m_MappedData;
        }

        void Buffer::Unmap()
        {
            if (m_Mapped)
            {
                vmaUnmapMemory(m_Device.GetMemoryAllocator(), m_Allocation);
                m_MappedData = nullptr;
                m_Mapped = false;
            }
        }

        void Buffer::Flush() const
        {
            vmaFlushAllocation(m_Device.GetMemoryAllocator(), m_Allocation, 0, m_Size);
        }
    }
}