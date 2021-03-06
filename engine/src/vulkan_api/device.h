#pragma once

#include "vulkan_api/physical_device.h"
#include "vulkan_api/resource_cache.h"
#include "vulkan_api/queue.h"
#include "vulkan_api/queue_family.h"

namespace engine
{
    class CommandPool;
    class FencePool;
    class Platform;

    class Device
    {
    public:
        Device(PhysicalDevice &gpu, Platform &platform,
               std::unordered_map<const char *, bool>
                   requested_extensions = {});
        ~Device();

        VkResult WaitIdle();
        bool IsExtensionEnabled(const char *extension);
        bool IsExtensionSupported(const char *requested_extension) const;
        bool IsImageFormatSupported(VkFormat format) const;
        const QueueFamily &GetSuitableGraphicsQueueFamily();

        QueueFamily &GetQueueFamilyByFlags(VkQueueFlags required_queue_flags);
        void CheckIfPresentSupported(VkSurfaceKHR surface);
        VkFence RequestFence();
        CommandBuffer &RequestCommandBuffer();

        VkDevice GetHandle() const { return m_Handle; }
        PhysicalDevice &GetGPU() const { return m_Gpu; }
        VmaAllocator GetMemoryAllocator() const { return m_MemoryAllocator; }
        ResourceCache &GetResourceCache() { return m_ResourceCache; }
        std::vector<QueueFamily> &GetQueueFamilies() { return m_QueueFamilies; }
        CommandPool &GetCommandPool() { return *m_CommandPool; }
        FencePool &GetFencePool() { return *m_FencePool; }

    private:
        PhysicalDevice &m_Gpu;
        Platform &m_Platform;
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
