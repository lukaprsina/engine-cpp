#pragma once

#include "vulkan_api/instance/physical_device.h"
#include "vulkan_api/instance/resource_cache.h"
#include "vulkan_api/instance/queue.h"

namespace engine
{
    class CommandPool;
    class FencePool;

    class Device
    {
    public:
        Device(PhysicalDevice &gpu, VkSurfaceKHR surface, std::unordered_map<const char *, bool> requested_extensions = {});
        ~Device();

        VkResult WaitIdle();
        bool IsExtensionSupported(const char *requested_extension) const;
        VkDevice GetHandle() const { return m_Handle; }
        QueueFamily &GetQueueByFlags(VkQueueFlags required_queue_flags, uint32_t queue_index);

    private:
        PhysicalDevice &m_Gpu;
        ResourceCache m_ResourceCache;

        std::vector<VkExtensionProperties> m_DeviceExtensions;
        std::vector<const char *> m_EnabledExtensions;

        VkDevice m_Handle = VK_NULL_HANDLE;
        std::vector<QueueFamily> m_QueueFamilies;
        VmaAllocator m_MemoryAllocator;

        std::unique_ptr<CommandPool> m_CommandPool;
        std::unique_ptr<FencePool> m_FencePool;
    };
}
