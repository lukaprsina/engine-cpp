#pragma once

#include "vulkan_api/instance/instance.h"

namespace engine
{
    class PhysicalDevice
    {
    public:
        PhysicalDevice(Instance &instance, VkPhysicalDevice physical_device);
        ~PhysicalDevice() = default;

        VkPhysicalDeviceFeatures GetFeatures() const { return m_Features; }
        VkPhysicalDeviceProperties GetProperties() const { return m_Properties; }
        VkPhysicalDeviceMemoryProperties GetMemoryProperties() const { return m_MemoryProperties; }

    private:
        Instance &m_Instance;
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceFeatures m_Features{};
        VkPhysicalDeviceProperties m_Properties{};
        VkPhysicalDeviceMemoryProperties m_MemoryProperties{};

        std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties{};
    };
}