#include "vulkan_api/instance/fence_pool.h"

#include "vulkan_api/instance/device.h"

namespace engine
{
    FencePool::FencePool(Device &device)
        : m_Device(device)
    {
    }

    FencePool::~FencePool()
    {
    }
}