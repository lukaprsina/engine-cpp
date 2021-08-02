#include "vulkan_api/queue_family.h"

#include "vulkan_api/device.h"
#include "vulkan_api/queue.h"

namespace engine
{
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
            m_Queues.emplace_back(device, *this, queue_index);
        }
    }
}
