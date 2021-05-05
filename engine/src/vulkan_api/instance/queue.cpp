#include "vulkan_api/instance/queue.h"
#include "vulkan_api/instance/device.h"

namespace engine
{
    Queue::Queue(Device &device,
                 uint32_t family_index,
                 VkQueueFamilyProperties properties,
                 VkBool32 can_present,
                 uint32_t index)
        : m_Device(device),
          m_FamilyIndex(family_index),
          m_Index(index),
          m_CanPresent(can_present),
          m_Properties(properties)
    {
        vkGetDeviceQueue(device.GetHandle(), family_index, index, &m_Handle);
    }
}