#include "vulkan_api/resource_cache.h"

#include "vulkan_api/device.h"

namespace engine
{
    ResourceCache::ResourceCache(Device &device)
        : m_Device(device)
    {
    }

    ResourceCache::~ResourceCache()
    {
    }
}
