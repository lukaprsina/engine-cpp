#pragma once

#include "vulkan_api/instance.h"

namespace engine
{
    class PhysicalDevice
    {
    public:
        PhysicalDevice(Instance &instance, VkPhysicalDevice physical_device);
        ~PhysicalDevice();

        VkPhysicalDeviceFeatures GetFeatures() const { return m_Features; }
        VkPhysicalDeviceFeatures GetRequestedFeatures() const { return m_RequestedFeatures; }
        VkPhysicalDeviceFeatures &GetMutableRequestedFeatures() { return m_RequestedFeatures; }
        VkPhysicalDeviceProperties GetProperties() const { return m_Properties; }
        VkPhysicalDeviceMemoryProperties GetMemoryProperties() const { return m_MemoryProperties; }

        const std::vector<VkQueueFamilyProperties> &GetQueueFamilyProperties() const { return m_QueueFamilyProperties; }
        VkPhysicalDevice GetHandle() const { return m_Handle; };
        Instance &GetInstance() const { return m_Instance; };
        VkBool32 IsPresentSupported(VkSurfaceKHR surface, uint32_t queue_family_index);

    private:
        Instance &m_Instance;
        VkPhysicalDevice m_Handle{VK_NULL_HANDLE};
        VkPhysicalDeviceFeatures m_Features{};
        VkPhysicalDeviceFeatures m_RequestedFeatures{};
        VkPhysicalDeviceProperties m_Properties{};
        VkPhysicalDeviceMemoryProperties m_MemoryProperties{};

        std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties{};
    };
}