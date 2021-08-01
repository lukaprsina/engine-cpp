#include "vulkan_api/command_buffer.h"

#include "vulkan_api/command_pool.h"
#include "vulkan_api/device.h"

namespace engine
{
    CommandBuffer::CommandBuffer(CommandPool &command_pool, VkCommandBufferLevel level)
        : m_Level{level},
          m_CommandPool{command_pool},
          m_MaxPushConstantsSize{command_pool.GetDevice().GetGPU().GetProperties().limits.maxPushConstantsSize}
    {
        VkCommandBufferAllocateInfo allocate_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};

        allocate_info.commandPool = command_pool.GetHandle();
        allocate_info.commandBufferCount = 1;
        allocate_info.level = level;

        VkResult result = vkAllocateCommandBuffers(command_pool.GetDevice().GetHandle(), &allocate_info, &m_Handle);

        if (result != VK_SUCCESS)
        {
            throw VulkanException{result, "Failed to allocate command buffer"};
        }
    }

    CommandBuffer::~CommandBuffer()
    {
        if (m_Handle != VK_NULL_HANDLE)
        {
            vkFreeCommandBuffers(m_CommandPool.GetDevice().GetHandle(),
                                 m_CommandPool.GetHandle(), 1, &m_Handle);
        }
    }

    VkResult CommandBuffer::Reset(ResetMode reset_mode)
    {
        VkResult result = VK_SUCCESS;

        assert(reset_mode == m_CommandPool.GetResetMode() && "Command buffer reset mode must match the one used by the pool to allocate it");

        m_State = State::Initial;

        if (reset_mode == ResetMode::ResetIndividually)
        {
            result = vkResetCommandBuffer(m_Handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        }

        return result;
    }
}
