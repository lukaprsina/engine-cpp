#include "vulkan_api/instance/physical_device.h"
#include "vulkan_api/instance/instance.h"

namespace engine
{
    PhysicalDevice::PhysicalDevice(Instance &instance, VkPhysicalDevice physical_device)
        : m_Instance(instance), m_PhysicalDevice(physical_device)
    {
        vkGetPhysicalDeviceFeatures(physical_device, &m_Features);
        vkGetPhysicalDeviceProperties(physical_device, &m_Properties);
        vkGetPhysicalDeviceMemoryProperties(physical_device, &m_MemoryProperties);

        ENG_CORE_TRACE("Found GPU: {}", m_Properties.deviceName);

        uint32_t queue_family_properties_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, nullptr);
        m_QueueFamilyProperties.resize(queue_family_properties_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, m_QueueFamilyProperties.data());
    }
}