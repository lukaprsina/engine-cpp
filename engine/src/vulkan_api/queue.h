#pragma once

namespace engine
{
    class Device;
    class CommandBuffer;
    class QueueFamily;

    class Queue
    {
    public:
        Queue(Device &device, uint32_t queue_family_index, uint32_t index);
        ~Queue() = default;

        VkResult Submit(const CommandBuffer &command_buffer, VkFence fence) const;
        VkResult Submit(const std::vector<VkSubmitInfo> &submit_infos, VkFence fence) const;

        VkResult Present(const VkPresentInfoKHR &present_infos) const;

        uint32_t GetQueueFamilyIndex() const { return m_QueueFamilyIndex; }
        uint32_t GetIndex() const { return m_Index; }
        VkQueue GetHandle() const { return m_Handle; }

    private:
        Device &m_Device;
        uint32_t m_QueueFamilyIndex{0};
        uint32_t m_Index{0};
        VkQueue m_Handle{VK_NULL_HANDLE};
    };
}