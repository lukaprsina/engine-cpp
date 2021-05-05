#pragma once

namespace engine
{
    class Device;
    class FencePool
    {
    public:
        FencePool(Device &device);
        ~FencePool();
    };
}