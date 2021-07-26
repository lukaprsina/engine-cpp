#include "vulkan_api/command_pool.h"

#include "vulkan_api/device.h"

namespace engine
{
    CommandPool::CommandPool(Device &device,
                             uint32_t queue_family_index)
        : m_Device(device), m_QueueFamilyIndex(queue_family_index)
    {
        VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        VkCommandPoolCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.queueFamilyIndex = queue_family_index;
        create_info.flags = flags;

        VkResult result = vkCreateCommandPool(device.GetHandle(), &create_info, nullptr, &m_Handle);
        if (result != VK_SUCCESS)
            throw VulkanException(result, "Failed to create command pool");
    }

    CommandPool::~CommandPool()
    {
        if (m_Handle != VK_NULL_HANDLE)
            vkDestroyCommandPool(m_Device.GetHandle(), m_Handle, nullptr);
    }
}