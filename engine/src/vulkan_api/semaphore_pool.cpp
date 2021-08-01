#include "vulkan_api/semaphore_pool.h"

namespace engine
{
    SemaphorePool::SemaphorePool(Device &device)
        : m_Device(device)
    {
    }

    SemaphorePool::~SemaphorePool()
    {
        Reset();

        for (VkSemaphore semaphore : m_Semaphores)
            vkDestroySemaphore(m_Device.GetHandle(), semaphore, nullptr);

        m_Semaphores.clear();
    }

    VkSemaphore SemaphorePool::RequestSemaphoreWithOwnership()
    {
        if (m_ActiveSemaphoreCount < m_Semaphores.size())
        {
            VkSemaphore semaphore = m_Semaphores.back();
            m_Semaphores.pop_back();
            return semaphore;
        }

        VkSemaphore semaphore = VK_NULL_HANDLE;
        VkSemaphoreCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkResult result = vkCreateSemaphore(m_Device.GetHandle(), &create_info, nullptr, &semaphore);

        if (result != VK_SUCCESS)
            throw std::runtime_error("Failed to create semaphore.");

        return semaphore;
    }

    void SemaphorePool::Reset()
    {
        m_ActiveSemaphoreCount = 0;

        for (auto &semaphore : m_ReleasedSemaphores)
            m_Semaphores.push_back(semaphore);

        m_ReleasedSemaphores.clear();
    }
}
