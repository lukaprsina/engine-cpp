#include "vulkan_api/fence_pool.h"

#include "vulkan_api/device.h"

namespace engine
{
    FencePool::FencePool(Device &device)
        : m_Device(device)
    {
    }

    FencePool::~FencePool()
    {
        Wait();
        Reset();

        for (VkFence fence : m_Fences)
            vkDestroyFence(m_Device.GetHandle(), fence, nullptr);

        m_Fences.clear();
    }

    VkFence FencePool::RequestFence()
    {
        // Check if there is an available fence
        if (m_ActiveFenceCount < m_Fences.size())
        {
            return m_Fences.at(m_ActiveFenceCount++);
        }

        VkFence fence{VK_NULL_HANDLE};

        VkFenceCreateInfo create_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};

        VkResult result = vkCreateFence(m_Device.GetHandle(), &create_info, nullptr, &fence);

        if (result != VK_SUCCESS)
            throw std::runtime_error("Failed to create fence.");

        m_Fences.push_back(fence);

        m_ActiveFenceCount++;

        return m_Fences.back();
    }

    VkResult FencePool::Wait(uint32_t timeout) const
    {
        if (m_ActiveFenceCount < 1 || m_Fences.empty())
        {
            return VK_SUCCESS;
        }

        return vkWaitForFences(m_Device.GetHandle(), m_ActiveFenceCount, m_Fences.data(), true, timeout);
    }

    VkResult FencePool::Reset()
    {
        if (m_ActiveFenceCount < 1 || m_Fences.empty())
        {
            return VK_SUCCESS;
        }

        VkResult result = vkResetFences(m_Device.GetHandle(), m_ActiveFenceCount, m_Fences.data());

        if (result != VK_SUCCESS)
        {
            return result;
        }

        m_ActiveFenceCount = 0;

        return VK_SUCCESS;
    }
}