#pragma once

#include "vulkan_api/device.h"

namespace engine
{
    class SemaphorePool
    {
    public:
        SemaphorePool(Device &device);
        ~SemaphorePool();

        VkSemaphore RequestSemaphoreWithOwnership();

    private:
        Device &m_Device;
    };
}
