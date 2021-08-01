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