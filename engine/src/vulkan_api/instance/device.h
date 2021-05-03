#pragma once

#include "vulkan_api/instance/physical_device.h"
#include "vulkan_api/instance/resource_cache.h"
#include "vulkan_api/instance/queue.h"
#include "common/vulkan_common.h"

namespace engine
{
    class Device
    {
    public:
        Device(PhysicalDevice &gpu, VkSurfaceKHR surface, std::unordered_map<const char *, bool> requested_extensions = {});
        ~Device();

        VkResult WaitIdle();
        bool IsExtensionSupported(const char *requested_extension) const;

    private:
        PhysicalDevice &m_Gpu;
        ResourceCache m_ResourceCache;

        std::vector<VkExtensionProperties> m_DeviceExtensions;
        std::vector<const char *> m_EnabledExtensions;

        VkDevice m_Handle = VK_NULL_HANDLE;

        std::vector<std::vector<Queue>> m_Queues;
    };
}