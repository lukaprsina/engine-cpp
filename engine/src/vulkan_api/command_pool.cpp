#include "vulkan_api/command_pool.h"

#include "vulkan_api/device.h"
#include "vulkan_api/render_frame.h"

namespace engine
{
    CommandPool::CommandPool(Device &device, uint32_t queue_family_index,
                             RenderFrame *render_frame, size_t thread_index,
                             CommandBuffer::ResetMode reset_mode)
        : m_Device(device), m_RenderFrame(render_frame),
          m_ThreadIndex(thread_index), m_ResetMode(reset_mode)
    {
        VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        switch (reset_mode)
        {
        case CommandBuffer::ResetMode::ResetIndividually:
        case CommandBuffer::ResetMode::AlwaysAllocate:
            flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            break;
        case CommandBuffer::ResetMode::ResetPool:
        default:
            flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            break;
        }

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
        m_PrimaryCommandBuffers.clear();
        m_SecondaryCommandBuffers.clear();

        if (m_Handle != VK_NULL_HANDLE)
            vkDestroyCommandPool(m_Device.GetHandle(), m_Handle, nullptr);
    }

    VkResult CommandPool::ResetPool()
    {
        VkResult result = VK_SUCCESS;

        switch (m_ResetMode)
        {
        case CommandBuffer::ResetMode::ResetIndividually:
        {
            result = ResetCommandBuffers();

            break;
        }
        case CommandBuffer::ResetMode::ResetPool:
        {
            result = vkResetCommandPool(m_Device.GetHandle(), m_Handle, 0);

            if (result != VK_SUCCESS)
            {
                return result;
            }

            result = ResetCommandBuffers();

            break;
        }
        case CommandBuffer::ResetMode::AlwaysAllocate:
        {
            m_PrimaryCommandBuffers.clear();
            m_ActivePrimaryCommandBufferCount = 0;

            m_SecondaryCommandBuffers.clear();
            m_ActiveSecondaryCommandBufferCount = 0;

            break;
        }
        default:
            throw std::runtime_error("Unknown reset mode for command pools");
        }

        return result;
    }

    VkResult CommandPool::ResetCommandBuffers()
    {
        VkResult result = VK_SUCCESS;

        for (auto &cmd_buf : m_PrimaryCommandBuffers)
        {
            result = cmd_buf->Reset(m_ResetMode);

            if (result != VK_SUCCESS)
            {
                return result;
            }
        }

        m_ActivePrimaryCommandBufferCount = 0;

        for (auto &cmd_buf : m_SecondaryCommandBuffers)
        {
            result = cmd_buf->Reset(m_ResetMode);

            if (result != VK_SUCCESS)
            {
                return result;
            }
        }

        m_ActiveSecondaryCommandBufferCount = 0;

        return result;
    }

    CommandBuffer &CommandPool::RequestCommandBuffer(VkCommandBufferLevel level)
    {
        if (level == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
        {
            if (m_ActivePrimaryCommandBufferCount < m_PrimaryCommandBuffers.size())
            {
                return *m_PrimaryCommandBuffers.at(m_ActivePrimaryCommandBufferCount++);
            }

            m_PrimaryCommandBuffers.emplace_back(std::make_unique<CommandBuffer>(*this, level));

            m_ActivePrimaryCommandBufferCount++;

            return *m_PrimaryCommandBuffers.back();
        }
        else
        {
            if (m_ActiveSecondaryCommandBufferCount < m_SecondaryCommandBuffers.size())
            {
                return *m_SecondaryCommandBuffers.at(m_ActiveSecondaryCommandBufferCount++);
            }

            m_SecondaryCommandBuffers.emplace_back(std::make_unique<CommandBuffer>(*this, level));

            m_ActiveSecondaryCommandBufferCount++;

            return *m_SecondaryCommandBuffers.back();
        }
    }
}