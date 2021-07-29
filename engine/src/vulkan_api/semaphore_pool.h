#pragma once

#include "vulkan_api/device.h"

namespace engine
{
    class SemaphorePool
    {
    public:
        SemaphorePool(Device &device);
        ~SemaphorePool();

    private:
        Device &m_Device;
    };
}
