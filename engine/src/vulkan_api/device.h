#pragma once

#include "vulkan_api/physical_device.h"
#include "vulkan_api/resource_cache.h"
#include "vulkan_api/queue.h"
#include "vulkan_api/queue_family.h"

namespace engine
{
    class CommandPool;
    class FencePool;

    class Device
    {
    public:
        Device(PhysicalDevice &gpu, VkSurfaceKHR surface,
               std::unordered_map<const char *, bool> requested_extensions = {});
        ~Device();

        VkResult WaitIdle();
        bool IsExtensionEnabled(const char *extension);
        bool IsExtensionSupported(const char *requested_extension) const;
        bool IsImageFormatSupported(VkFormat format) const;
        const QueueFamily &GetSuitableGraphicsQueueFamily();

        VkDevice GetHandle() const { return m_Handle; }
        PhysicalDevice GetGPU() const { return m_Gpu; }
        VmaAllocator GetMemoryAllocator() const { return m_MemoryAllocator; }
        ResourceCache &GetResourceCache() { return m_ResourceCache; }

        QueueFamily &GetQueueFamilyByFlags(VkQueueFlags required_queue_flags);

    private:
        PhysicalDevice &m_Gpu;
        ResourceCache m_ResourceCache;

        std::vector<VkExtensionProperties> m_DeviceExtensions{};
        std::vector<const char *> m_EnabledExtensions{};

        VkDevice m_Handle{VK_NULL_HANDLE};
        std::vector<QueueFamily> m_QueueFamilies{};
        VmaAllocator m_MemoryAllocator{};

        std::unique_ptr<CommandPool> m_CommandPool{};
        std::unique_ptr<FencePool> m_FencePool{};
    };
}
