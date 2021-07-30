#include "vulkan_api/semaphore_pool.h"

namespace engine
{
    SemaphorePool::SemaphorePool(Device &device)
        : m_Device(device)
    {
    }

    SemaphorePool::~SemaphorePool()
    {
    }

    VkSemaphore RequestSemaphoreWithOwnership()
    {
        }
}
