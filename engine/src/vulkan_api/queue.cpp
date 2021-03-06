#include "vulkan_api/queue.h"

#include "vulkan_api/device.h"
#include "vulkan_api/command_buffer.h"
#include "vulkan_api/queue_family.h"

namespace engine
{
    Queue::Queue(Device &device, uint32_t queue_family_index, uint32_t index)
        : m_Device(device), m_QueueFamilyIndex(queue_family_index), m_Index(index)

    {
        vkGetDeviceQueue(device.GetHandle(), queue_family_index, index, &m_Handle);
    }

    VkResult Queue::Submit(const CommandBuffer &command_buffer, VkFence fence) const
    {
        VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer.GetHandle();

        return Submit({submit_info}, fence);
    }

    VkResult Queue::Submit(const std::vector<VkSubmitInfo> &submit_infos, VkFence fence) const
    {
        return vkQueueSubmit(m_Handle, ToUint32_t(submit_infos.size()), submit_infos.data(), fence);
    }

    VkResult Queue::Present(const VkPresentInfoKHR &present_info) const
    {
        if (!m_Device.GetQueueFamilies()[m_QueueFamilyIndex].CanPresent())
        {
            return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;
        }

        return vkQueuePresentKHR(m_Handle, &present_info);
    }
}