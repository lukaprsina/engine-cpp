#include "vulkan_api/resource_cache.h"

#include "vulkan_api/device.h"
#include "common/resource_caching.h"

namespace engine
{
    namespace
    {
        template <class T, class... A>
        T &RequestResource(Device &device, ResourceRecord &recorder,
                           std::mutex &resource_mutex, entt::resource_cache<T> &cache,
                           A &...args)
        {
            std::lock_guard<std::mutex> guard(resource_mutex);

            auto &res = request_resource(device, &recorder, resources, args...);

            return res;
        }
    }

    ResourceCache::ResourceCache(Device &device)
        : m_Device(device)
    {
    }

    ResourceCache::~ResourceCache()
    {
    }
}
