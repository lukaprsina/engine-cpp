#pragma once

namespace engine
{
    class Device;
    class FencePool
    {
    public:
        FencePool(Device &device);
        ~FencePool();

        VkResult Wait(uint32_t timeout = std::numeric_limits<uint32_t>::max()) const;
        VkResult Reset();

    private:
        Device &m_Device;
        std::vector<VkFence> m_Fences{};
        uint32_t m_ActiveFenceCount{0};
    };
}