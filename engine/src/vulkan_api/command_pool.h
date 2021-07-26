#pragma once

namespace engine
{
    class Device;
    class CommandPool
    {
    public:
        CommandPool(Device &device,
                    uint32_t queue_family_index);
        ~CommandPool();

    private:
        Device &m_Device;
        uint32_t m_QueueFamilyIndex;
        VkCommandPool m_Handle = VK_NULL_HANDLE;
    };
}