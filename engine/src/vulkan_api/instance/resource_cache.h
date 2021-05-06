#pragma once

namespace engine
{
    class Device;
    class ResourceCache
    {
    public:
        ResourceCache(Device &device);
        ResourceCache(const ResourceCache &) = delete;
        ResourceCache(ResourceCache &&) = delete;
        ~ResourceCache();
        ResourceCache &operator=(const ResourceCache &) = delete;
        ResourceCache &operator=(ResourceCache &&) = delete;
    };
}