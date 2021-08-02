#pragma once

#include "vulkan_api/device.h"

namespace engine
{
    class SemaphorePool
    {
    public:
        SemaphorePool(Device &device);
        ~SemaphorePool();

        VkSemaphore RequestSemaphore();
        VkSemaphore RequestSemaphoreWithOwnership();
        void ReleaseOwnedSemaphore(VkSemaphore semaphore);

        void Reset();

    private:
        Device &m_Device;
        std::vector<VkSemaphore> m_Semaphores{};
        std::vector<VkSemaphore> m_ReleasedSemaphores{};
        uint32_t m_ActiveSemaphoreCount{0};
    };
}
