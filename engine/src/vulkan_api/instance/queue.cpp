#include "vulkan_api/instance/queue.h"
#include "vulkan_api/instance/device.h"

namespace engine
{
    Queue::Queue(Device &device, uint32_t family_index, uint32_t index)
        : m_Device(device), m_FamilyIndex(family_index), m_Index(index)

    {
        vkGetDeviceQueue(device.GetHandle(), family_index, index, &m_Handle);
    }

    QueueFamily::QueueFamily(Device &device,
                             uint32_t family_index,
                             VkQueueFamilyProperties properties,
                             VkBool32 can_present)
        : m_Device(device),
          m_FamilyIndex(family_index),
          m_CanPresent(can_present),
          m_Properties(properties)
    {
        for (uint32_t queue_index = 0; queue_index < properties.queueCount; queue_index++)
        {
            m_Queues.emplace_back(device, family_index, queue_index);
        }
    }

}