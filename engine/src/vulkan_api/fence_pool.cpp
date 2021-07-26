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
    }
}